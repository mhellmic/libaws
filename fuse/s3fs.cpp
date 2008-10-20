/*
 * Copyright 2008 28msec, Inc.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/**
 * mapping to s3
 * directory:
 *   - empty object with the name of the dir, e.g. /testdir
 *   - metadata: //saved in the object 
 *     - dir : 1
 *     - mode : int
 *     - gid : int
 *     - oid : int
 *     - mtime : long 
 * file:
 *  - object with the path/name of the file, e.g. /testdir/testfile
 *  - metadata:
 *     - file: 1
 *     - mode : int
 *     - gid : int
 *     - oid : int
 *     - mtime : long 
 */
#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION  26
#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <map>
#include <sstream>
#include <fstream>
#include <iostream>

#include <libaws/aws.h>

using namespace aws;


static AWSConnectionFactory* theFactory;
static std::string theAccessKeyId;
static std::string theSecretAccessKey;
static std::string theS3FSTempFolder;
static std::string BUCKETNAME("msb");
static std::map<int,std::string> tempfilenamemap;
#ifndef NDEBUG
static int S3_DEBUG=0;
static int S3_INFO=1;
static int S3_ERROR=2;
static int S3_LOGGING_LEVEL=S3_DEBUG;
#endif

/**
 * accessor and release functions for S3 Connection objects
 */
static S3ConnectionPtr getConnection() {
  return theFactory->createS3Connection(theAccessKeyId, theSecretAccessKey);
}

static void releaseConnection(const S3ConnectionPtr& aConnection) {
}

/**
 * macro that should be used to exit a function
 * it releases the connection object used in the function and returns with the given return code
 */
#define S3FS_EXIT(code) \
  releaseConnection(lCon); \
  return code;

/**
 * macros that should be used for try-catch combinations
 */
#define S3FS_TRY try {

#ifndef NDEBUG
#  define S3FS_CATCH(kind) \
    } catch (kind ## Exception & s3Exception) { \
      S3_LOG(S3_ERROR,location,s3Exception.what()); \
      S3FS_EXIT(-ENOENT); \
    } catch (AWSConnectionException & conException) { \
     S3_LOG(S3_ERROR,location,conException.what()); \
     S3FS_EXIT(-ECONNREFUSED); \
    }catch (AWSException & awsException) { \
        S3_LOG(S3_ERROR,location,awsException.what()); \
        S3FS_EXIT(-ENOENT); \
    }
#else
#  define S3FS_CATCH(kind) \
    } catch (kind ## Exception & s3Exception) { \
      S3_LOG(S3_ERROR,location,s3Exception.what()); \
      S3FS_EXIT(-ENOENT); \
    } catch (AWSConnectionException & conException) { \
        S3_LOG(S3_ERROR,location,conException.what()); \
      S3FS_EXIT(-ECONNREFUSED); \
    }catch (AWSException & awsException) { \
        S3_LOG(S3_ERROR,location,awsException.what()); \
        S3FS_EXIT(-ENOENT); \
    }
#endif

// shorcuts
typedef std::map<std::string, std::string> map_t;
typedef std::pair<std::string, std::string> pair_t;

/** 
 * helper functions
 */
static mode_t
to_int(const std::string s)
{
  return (mode_t) atoi(s.c_str());
}

static std::string
to_string(int i)
{
  std::stringstream s;
  s << i;
  return s.str();
}

static void
fill_stat(map_t& aMap, struct stat* stbuf, long long aContentLength)
{
  stbuf->st_mode |= to_int(aMap["mode"]);
  stbuf->st_gid  |= to_int(aMap["gid"]);
  stbuf->st_uid  |= to_int(aMap["oid"]);
  stbuf->st_mtime  |= to_int(aMap["mtime"]);
  stbuf->st_size = aContentLength;

  if (aMap.count("dir") != 0) {
    stbuf->st_mode |= S_IFDIR;
    stbuf->st_nlink = 2;
  } else if (aMap.count("file") != 0) {
    stbuf->st_mode |= S_IFREG;
    stbuf->st_nlink = 1;
  } 
  //stbuf->st_atime = 0
  //stbuf->st_ctime = 0


}

#ifndef NDEBUG

#define S3_LOG(level,location,message) \
   if(S3_LOGGING_LEVEL <= level) \
   { \
	std::ostringstream logMessage; \
	std::string levelstr=""; \
	if (level==S3_DEBUG){ \
		levelstr="DEBUG"; \
	}else if (level==S3_INFO){ \
		levelstr="INFO"; \
	}else if (level==S3_ERROR){ \
		levelstr="ERROR"; \
	} \
	logMessage << "[" << levelstr << "] " << location << " ## " << message << " ##";  \
	std::cerr << logMessage.str() << std::endl; \
   } 
#else
#define S3_LOG(level,location,message)
#endif

/** 
 * Get file/folder attributes.
 * 
 * Similar to stat(). The 'st_dev' and 'st_blksize' fields are ignored. 
 * The 'st_ino' field is ignored except if the 'use_ino' mount option is given.
 * 
 */
static int
s3_getattr(const char *path, struct stat *stbuf)
{
#ifndef NDEBUG
  std::string location="s3_getattr(...)";
  S3_LOG(S3_DEBUG,location,"getattr path: " << path);
#endif

  // initialize result
  int result=0;
  memset(stbuf, 0, sizeof(struct stat));

  // we always immediately exit if the root dir is requested.
  if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
    stbuf->st_mode = S_IFDIR | 0755;
    stbuf->st_nlink = 2;

    // set the meta data in the stat struct
    map_t lMap;
    lMap["mode"]="493";
    lMap["gid"]="0";
    lMap["uid"]="0";
    lMap["dir"]="1";
    fill_stat(lMap, stbuf, 4096);

#ifndef NDEBUG
    S3_LOG(S3_DEBUG,location,"  requested getattr for root / => exit");
#endif
    result=0;
  }else if (strcmp(path, "/libattr.so.1") == 0) { 
    S3_LOG(S3_DEBUG,location,"  refusing "<<path);
    result=-ENOENT;
  }else if (strcmp(path, "/libpthread.so.0") == 0) { 
    S3_LOG(S3_DEBUG,location,"  refusing "<<path);
    result=-ENOENT;
  }else if (strcmp(path, "/libacl.so.1") == 0) { 
    S3_LOG(S3_DEBUG,location,"  refusing "<<path);
    result=-ENOENT;
  }else if (strcmp(path, "/librt.so.1") == 0) { 
    S3_LOG(S3_DEBUG,location,"  refusing "<<path);
    result=-ENOENT;
  }else if (strcmp(path, "/libc.so.6") == 0) { 
    S3_LOG(S3_DEBUG,location,"  refusing "<<path);
    result=-ENOENT;
  }else if (strcmp(path, "/tls") == 0) { 
    S3_LOG(S3_DEBUG,location,"  refusing "<<path);
    result=-ENOENT;
  }else if (strcmp(path, "/i686") == 0) { 
    S3_LOG(S3_DEBUG,location,"  refusing "<<path);
    result=-ENOENT;
  }else if (strcmp(path, "/sse2") == 0) { 
    S3_LOG(S3_DEBUG,location,"  refusing "<<path);
    result=-ENOENT;
  }else{

    S3ConnectionPtr lCon = getConnection();
    S3FS_TRY

       std::string lpath(path);

       // check if we have that path without first /
       HeadResponsePtr lRes;
       lRes = lCon->head(BUCKETNAME, lpath.substr(1));
       map_t lMap = lRes->getMetaData();
#ifndef NDEBUG
       S3_LOG(S3_DEBUG,location,"  requested metadata for " << lpath.substr(1));

       // get information without first /
       for (map_t::iterator lIter = lMap.begin(); lIter != lMap.end(); ++lIter) {
          S3_LOG(S3_DEBUG,location,"    got " << (*lIter).first << " : " << (*lIter).second);
       }
       S3_LOG(S3_DEBUG,location,"    content-length: " << lRes->getContentLength());
#endif

       // set the meta data in the stat struct
       fill_stat(lMap, stbuf, lRes->getContentLength());
    S3FS_CATCH(Head)

    S3FS_EXIT(result);
  }

  return result;
}



static int
s3_mkdir(const char *path, mode_t mode)
{
#ifndef NDEBUG
  std::string location="s3_mkdir(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " mode: " << mode);
#endif

  S3ConnectionPtr lCon = getConnection();
  S3FS_TRY
    map_t lDirMap;
    lDirMap.insert(pair_t("dir", "1"));
    lDirMap.insert(pair_t("gid", to_string(getgid())));
    lDirMap.insert(pair_t("uid", to_string(getuid())));
    lDirMap.insert(pair_t("mode", to_string(mode)));
    PutResponsePtr lRes = lCon->put(BUCKETNAME, path, 0, "text/plain", 0, &lDirMap);
    S3FS_EXIT(0);
  S3FS_CATCH(Put)
  S3FS_EXIT(-ENOENT);
}



static int
s3_rmdir(const char *path)
{
#ifndef NDEBUG
  std::string location="s3_rmdir(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path);
#endif

  S3ConnectionPtr lCon = getConnection();

  try {
    HeadResponsePtr lRes;
    lRes = lCon->head(BUCKETNAME, path);
    map_t lMap = lRes->getMetaData();
    if (lMap.count("dir") == 0) {
#ifndef NDEBUG
      std::cerr << " not a directory" << std::endl;
#endif
      S3FS_EXIT(-ENOTTY);  
    }
  } catch (HeadException &e) {
    if (e.getErrorCode() == aws::S3Exception::NoSuchKey) {
#ifndef NDEBUG
      std::cerr << " no such key" << std::endl;
#endif
      S3FS_EXIT(-ENOENT);
    }
#ifndef NDEBU
    std::cerr << " something unexpected happened: " << e.getErrorMessage() << std::endl;
#endif
    S3FS_EXIT(-ENOENT);
  }

  S3FS_TRY
    DeleteResponsePtr lRes = lCon->del(BUCKETNAME, path);
  S3FS_CATCH(Put)
  S3FS_EXIT(0);
}


/*
 * Read directory
 * 
 * The filesystem may choose between two modes of operation:
 * 
 * 1) The readdir implementation ignores the offset parameter, and passes zero 
 * to the filler function's offset. The filler function will not return '1' 
 * (unless an error happens), so the whole directory is read in a single readdir 
 * operation. This works just like the old getdir() method.
 * 
 * 2) The readdir implementation keeps track of the offsets of the directory 
 * entries. It uses the offset parameter and always passes non-zero offset to
 * the filler function. When the buffer is full (or an error happens) the filler
 *  function will return '1'.
 * 
 */
static int
s3_readdir(const char *path,
           void *buf,
           fuse_fill_dir_t filler,
           off_t offset,
           struct fuse_file_info *fi)
{
#ifndef NDEBUG
  std::string location="s3_readdir(...)";
  S3_LOG(S3_DEBUG,location,"readdir: " << path);
#endif

  S3ConnectionPtr lCon = getConnection();

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);

  // TODO handle full buffer by remembering the marker

  std::string lPath(path);
  if (lPath.at(lPath.length()-1) != '/')
    lPath += "/";

  ListBucketResponsePtr lRes;
  S3FS_TRY
    std::string lMarker;
    do {
      // get object without first /
#ifndef NDEBUG
      S3_LOG(S3_DEBUG,location,"  list bucket: "<<BUCKETNAME<<" prefix: "<<lPath.substr(1));
#endif
      lRes = lCon->listBucket(BUCKETNAME, lPath.substr(1), lMarker, "/", -1);
      lRes->open();
      ListBucketResponse::Object o;
      while (lRes->next(o)) {
        struct stat lStat;
        memset(&lStat, 0, sizeof(struct stat));

#ifndef NDEBUG
        S3_LOG(S3_DEBUG,location,"  result: " << o.KeyValue);
#endif
        std::string lTmp = o.KeyValue.replace(0, lPath.length()-1, "");
        int isfull=filler(buf, lTmp.c_str(), &lStat, 0);
#ifndef NDEBUG
        if(isfull){
           S3_LOG(S3_INFO,location,"  buffer is full");
        }
#endif
        lMarker = o.KeyValue;
      }
      lRes->close();
    } while (lRes->isTruncated());

    S3FS_EXIT(0);
  S3FS_CATCH(ListBucket);

  S3FS_EXIT(-ENOENT);
}



static int
s3_create(const char *path, mode_t mode, struct fuse_file_info *fs)
{
#ifndef NDEBUG
  std::string location="s3_create(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " mode: " << mode);
#endif


  S3ConnectionPtr lCon = getConnection();
  S3FS_TRY
    map_t lDirMap;
    lDirMap.insert(pair_t("file", "1"));
    lDirMap.insert(pair_t("gid", to_string(getgid())));
    lDirMap.insert(pair_t("uid", to_string(getuid())));
    lDirMap.insert(pair_t("mode", to_string(mode)));
    PutResponsePtr lRes = lCon->put(BUCKETNAME, path, 0, "text/plain", 0, &lDirMap);
    S3FS_EXIT(0);
  S3FS_CATCH(Put)
  S3FS_EXIT(-ENOENT);
}




// open, write, release
// always use a temporary file

/*
 *
 * File open operation
 * 
 * No creation, or truncation flags (O_CREAT, O_EXCL, O_TRUNC) will be passed
 * to open(). Open should check if the operation is permitted for the given 
 * flags. Optionally open may also return an arbitrary filehandle in the 
 * fuse_file_info structure, which will be passed to all file operations.
 */
static int
s3_open(const char *path, 
	struct fuse_file_info *fileinfo)
{
#ifndef NDEBUG
  std::string location="s3_open(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path);
#endif

  // initialize result
  int result=0;
  memset(fileinfo, 0, sizeof(struct fuse_file_info));

  // generate temp file and open it
  int ltempsize=theS3FSTempFolder.length();
  char ltempfile[ltempsize];
  strcpy(ltempfile,theS3FSTempFolder.c_str());
  int fileHandle = mkstemp(ltempfile);
#ifndef NDEBUG
  S3_LOG(S3_DEBUG,location,"File Descriptor # is: " << fileHandle << " file name = " << ltempfile);
#endif
  std::auto_ptr<std::fstream> tempfile(new std::fstream());
  tempfile->open(ltempfile);

  // now lets get the data and save it into the temp file
  S3ConnectionPtr lCon = getConnection();

  S3FS_TRY

    std::string lpath(path);
    GetResponsePtr lGet = lCon->get(BUCKETNAME, lpath.substr(1));
    std::istream& lInStream = lGet->getInputStream();

#ifndef NDEBUG
    S3_LOG(S3_DEBUG,location,"content length: " << lGet->getContentLength());
#endif

    // write data to temp file
    while (lInStream.good())     // loop while extraction from file is possible
    {
      (*tempfile) << (char) lInStream.get();       // get character from file
    }
    (*tempfile) << lInStream;
    tempfile->flush();

    //remember tempfile
    fileinfo->fh = (uint64_t)fileHandle;
    tempfilenamemap.insert( std::pair<int,std::string>(fileHandle,ltempfile) );

    //cleanup
    if(tempfile->is_open()) tempfile->close();

  S3FS_CATCH(Get)
    
  return result;
}


/*
 * Release the open tempfile
 * 
 * Release is called when there are no more references to an open 
 * file: all file descriptors are closed and all memory mappings are 
 * unmapped.
 * 
 * For every open() call there will be exactly one release() call with 
 * the same flags and file descriptor. It is possible to have a file 
 * opened more than once, in which case only the last release will mean, 
 * that no more reads/writes will happen on the file. The return value 
 * of release is ignored.
 * 
 */
static int
s3_release(const char *path, struct fuse_file_info *fileinfo)
{
#ifndef NDEBUG
  std::string location="s3_release(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path);
#endif

  // initialize result
  int result=0;
 
  // get name of temp file
  int fileHandle = (int)fileinfo->fh;
  std::string tempfilename=tempfilenamemap.find(fileHandle)->second;

  //cleanup 
  remove(tempfilename.c_str()); 
  tempfilenamemap.erase(fileHandle);

  return result;
}



/*
 * Read data from an open file
 * 
 * Read should return exactly the number of bytes requested except on EOF
 * or error, otherwise the rest of the data will be substituted with zeroes. 
 * An exception to this is when the 'direct_io' mount option is specified, 
 * in which case the return value of the read system call will reflect the 
 * return value of this operation.
 * 
 */
static int
s3_read(const char *path,
        char *buf,
        size_t size,
        off_t offset,
        struct fuse_file_info *fileinfo)
{
#ifndef NDEBUG
  std::string location="s3_read(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " offset: " << offset << " size: " << size);
#endif

  int fileHandle = (int)fileinfo->fh;
  std::string tempfilename=tempfilenamemap.find(fileHandle)->second;
  std::auto_ptr<std::fstream> tempfile(new std::fstream());
  tempfile->open(tempfilename.c_str());

  // get length of file:
  tempfile->seekg (0, std::ios::end);
  unsigned int filelength = tempfile->tellg();
  int readsize = 0;
  if(size>filelength || (size-offset)>filelength){
    readsize=filelength-offset;
  }else{
    readsize=size;
  }
  
  tempfile->seekg(offset);
  memset(buf, 0, readsize); 
  tempfile->read(buf,readsize);
  return readsize;
  
#if 0
  GetResponsePtr lRes;
  S3FS_TRY
    lRes = lCon->get(BUCKETNAME, path);
    S3FS_EXIT(0);
  S3FS_CATCH(Put)
#endif
}


/*
 * Open directory
 * 
 * This method should check if the open operation is permitted for this directory
 * 
 * it is called before readdir
 */

static int
s3_opendir(const char *path, struct fuse_file_info *fi)
{
#ifndef NDEBUG
  std::string location="s3_opendir(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path);
#endif

  // by default allow access
  return 0;
}



static struct fuse_operations s3_filesystem_operations;

int
main(int argc, char **argv)
{
  // set callback functions
  s3_filesystem_operations.getattr    = s3_getattr;
  s3_filesystem_operations.mkdir      = s3_mkdir;
  s3_filesystem_operations.rmdir      = s3_rmdir;
  s3_filesystem_operations.readdir    = s3_readdir;
  s3_filesystem_operations.create     = s3_create;
  s3_filesystem_operations.opendir     = s3_opendir;
  // can't be supported because s3 doesn't allow to change meta data without putting the object again
  s3_filesystem_operations.read       = s3_read;
  s3_filesystem_operations.open       = s3_open;
  s3_filesystem_operations.release       = s3_release;

  // initialization
  theFactory = AWSConnectionFactory::getInstance();

  if(getenv("S3FS_TEMP")!=NULL){
    theS3FSTempFolder = getenv("S3FS_TEMP");
    theS3FSTempFolder.append("/s3fs_file_XXXXXX");
  }else{
    std::cerr << "Please specify the S3FS_TEMP environment variable. It should point to a folder where you have write access." << std::endl;
  }
  if(getenv("AWS_ACCESS_KEY")!=NULL){
    theAccessKeyId = getenv("AWS_ACCESS_KEY");
  }else{
    theAccessKeyId = "";
  }
  if(getenv("AWS_SECRET_ACCESS_KEY")!=NULL){
    theSecretAccessKey = getenv("AWS_SECRET_ACCESS_KEY");
  }else{
    theSecretAccessKey = "";
  }

  if (theAccessKeyId.length() == 0) {
    std::cerr << "Please specify the AWS_ACCESS_KEY environment variable" << std::endl;
    return 1;
  }
  if (theSecretAccessKey.length() == 0) {
    std::cerr << "Please specify the AWS_SECRET_ACCESS_KEY environment variable" << std::endl;
    return 2;
  }

  std::cerr << "####################" << std::endl;
  std::cerr << "####################" << std::endl;
  std::cerr << "# S3FS starting up #" << std::endl;
  std::cerr << "####################" << std::endl;
  std::cerr << "####################" << std::endl;
    
  // let's get started
  return fuse_main(argc, argv, &s3_filesystem_operations, NULL);
}
