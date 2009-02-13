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
#define S3FS_LOG_SYSLOG 1

#include "config.h"

#include <fuse.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <map>
#include <sstream>
#include <syslog.h>
#include <fstream>
#include <cassert>
#include <stdlib.h>
#include <memory>
#include <cassert>

#include <libaws/aws.h>
#include "properties.h"

#ifdef S3FS_USE_MEMCACHED
#  include <libmemcached/memcached.h>
#  include "awscache.h"
#endif //USE_MEMCACHED

using namespace aws;

#ifdef S3FS_USE_MEMCACHED
static AWSCache* theCache;
#endif //USE_MEMCACHED

static AWSConnectionFactory* theFactory;
static ConnectionPool<S3ConnectionPtr>* theS3ConnectionPool;
static unsigned int CONNECTION_POOL_SIZE=5;
static unsigned int AWS_TRIES_ON_ERROR=3;

static std::string theAccessKeyId;
static std::string theSecretAccessKey;
static std::string theS3FSTempFolder;
static std::string theBucketname("");
static std::string thePropertyFile("");

static std::string DELIMITER_FOLDER_ENTRIES=",";

static std::map<int,struct FileHandle*> tempfilemap;

#ifndef NDEBUG
static int S3_DEBUG=0;
static int S3_INFO=1;
static int S3_ERROR=2;
static int S3_LOGGING_LEVEL=S3_INFO;
#endif

/**
 * FileHandle struct definition, constructor and destructor
 */

struct FileHandle {
   FileHandle();
   ~FileHandle();
   int id;
   std::fstream* filestream;
   std::string filename;
   std::string s3key;
   unsigned int size;
   bool is_write; 
   mode_t mode;
   time_t mtime;
};

FileHandle::FileHandle()
{
  id=-1;
  filestream=NULL;
  filename="";
  s3key="";
  size=0;
  is_write=false;
  mode=0;
  mtime=0;
}

FileHandle::~FileHandle()
{
  if(id!=-1){
     tempfilemap.erase(id);
     close(id);
  }
  if(filestream){
     delete filestream;filestream=0;
  }

  // delete tempfilename if existent
  if(!filename.empty()){
    remove(filename.c_str());
  }
}

/**
 * accessor and release functions for S3 Connection objects
 */
static S3ConnectionPtr getConnection() {
//  return theFactory->createS3Connection(theAccessKeyId, theSecretAccessKey);
  return theS3ConnectionPool->getConnection();
}

static void releaseConnection(const S3ConnectionPtr& aConnection) {
  theS3ConnectionPool->release(aConnection);
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
      S3_LOG(S3_ERROR,location, "S3Exception(ERRORCODE="<<((int)s3Exception.getErrorCode())<<"):"<<s3Exception.what()); \
      if (s3Exception.getErrorCode() != aws::S3Exception::NoSuchKey) { \
         haserror=true; \
      } else{ \
         haserror=false; \
      } \
      result=-ENOENT;\
    } catch (AWSConnectionException & conException) { \
     S3_LOG(S3_ERROR,location,"AWSConnectionException: "<<conException.what()); \
      haserror=true; \
      result=-ECONNREFUSED; \
    }catch (AWSException & awsException) { \
      S3_LOG(S3_ERROR,location,"AWSException: "<<awsException.what()); \
      haserror=true; \
      result=-ENOENT;\
    }
#else
#  define S3FS_CATCH(kind) \
    } catch (kind ## Exception & s3Exception) { \
      if (s3Exception.getErrorCode() != aws::S3Exception::NoSuchKey) { \
         haserror=true; \
      } else{ \
         haserror=false; \
      } \
      result=-ENOENT;\
    } catch (AWSConnectionException & conException) { \
      haserror=true; \
      result=-ECONNREFUSED; \
    }catch (AWSException & awsException) { \
      haserror=true; \
      result=-ENOENT;\
    }
#endif

#ifndef NDEBUG

#  ifdef S3FS_LOG_SYSLOG
#    define S3_LOG(level,location2,message) \
       if(S3_LOGGING_LEVEL <= level) \
       { \
	    std::ostringstream logMessage; \
            if (level==S3_DEBUG){ \
                    logMessage << "S3FS(bucket:" << theBucketname << ") " << location2 << " [DEBUG] ## " << message << " ## "; \
		    syslog( LOG_DEBUG, logMessage.str().c_str() ); \
	    }else if (level==S3_INFO){ \
                    logMessage << "S3FS(bucket:" << theBucketname << ") " << location2 << " [INFO] ## " << message << " ## "; \
		    syslog( LOG_NOTICE, logMessage.str().c_str() ); \
	    }else if (level==S3_ERROR){ \
                    logMessage << "S3FS(bucket:" << theBucketname << ") " << location2 << " [ERROR] ## " << message << " ## "; \
		    syslog( LOG_ERR, logMessage.str().c_str() ); \
	    } \
	    } 
#  else
#    define S3_LOG(level,location2,message) \
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
	    logMessage << "[" << levelstr << "] " << location2 << " ## " << message << " ##";  \
	    std::cerr << logMessage.str() << std::endl; \
       } 
#  endif
#else
#define S3_LOG(level,location2,message)
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
/*
static long
to_long(const std::string s){
  return strtol(s.c_str(),NULL,10);
}
*/
template <class T>
static std::string
to_string(T i)
{
  std::stringstream s;
  s << i;
  return s.str();
}

static time_t 
string_to_time(std::string timestring){
  int yy, mm, dd, hour, min, sec;
  struct tm timeinfo;
  time_t tme;

  sscanf(timestring.c_str(), "%d/%d/%d %d:%d:%d", &mm, &dd, &yy, &hour, &min, &sec);

  time(&tme);
  timeinfo = *localtime(&tme);
  timeinfo.tm_year = yy; 
  timeinfo.tm_mon = mm-1; 
  timeinfo.tm_mday = dd;
  timeinfo.tm_hour = hour; 
  timeinfo.tm_min = min; 
  timeinfo.tm_sec = sec;
  return( mktime(&timeinfo) );
}

static std::string
time_to_string(time_t rawtime){
  struct tm timeinfo;
  timeinfo = *localtime ( &rawtime );
  std::stringstream converter;
  converter << (timeinfo.tm_mon+1) << "/" << timeinfo.tm_mday << "/" << timeinfo.tm_year << " "
         << timeinfo.tm_hour << ":" << timeinfo.tm_min << ":" << timeinfo.tm_sec;
  return converter.str();
}

static time_t
getCurrentTime(){
  time_t rawtime;
  time ( &rawtime );
  return rawtime;
}

static void
fill_stat(map_t& aMap, struct stat* stbuf, long long aContentLength)
{
  stbuf->st_mode = to_int(aMap["mode"]);
  stbuf->st_gid  = to_int(aMap["gid"]);
  stbuf->st_uid  = to_int(aMap["oid"]);
  stbuf->st_mtime  = string_to_time(aMap["mtime"]);

  if (aMap.count("dir") != 0) {
    stbuf->st_mode |= S_IFDIR;
    stbuf->st_nlink = 2;
    stbuf->st_size = 4096;
  } else if (aMap.count("file") != 0) {
    stbuf->st_mode |= S_IFREG;
    stbuf->st_size = aContentLength;
    stbuf->st_nlink = 1;
  } 
  //stbuf->st_atime = 0
  //stbuf->st_ctime = 0
}


/**
 * Predeclarations
 */
static int
s3_release(const char *path, struct fuse_file_info *fileinfo);


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
  std::string location="s3_getattr";
#endif
  // initialize result
  int result=0;
  memset(stbuf, 0, sizeof(struct stat));
  std::string lpath(path);

  try{

    // we always immediately exit if the root dir is requested.
    if (strcmp(path, "/") == 0) { /* The root directory of our file system. */

      // set the meta data in the stat struct
      stbuf->st_mode = S_IFDIR | 0777;
      stbuf->st_gid  = getgid();
      stbuf->st_uid  = getuid();
      stbuf->st_mtime  = getCurrentTime();
      stbuf->st_nlink = 2;
      stbuf->st_size = 4096;

      S3_LOG(S3_DEBUG,"s3_getattr(...)","  requested getattr for root / => exit");
      return result;
    }else{

#ifdef S3FS_USE_MEMCACHED
      std::string value;

      memcached_return rc;

      // check if the cache knows if the file/folder exists
      std::string key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
      value=theCache->read_key(key, &rc);
      if (value.length() > 0 && value.compare("0")==0) // file does not exist
      {
        S3_LOG(S3_DEBUG,"s3_getattr(...)","[Memcached] file or folder: " << lpath.substr(1) << " is marked as non existent in cache.");
        return -ENOENT;
      }else if(value.length() > 0 && value.compare("1")==0) // file does exist
      {
        S3_LOG(S3_DEBUG,"s3_getattr(...)","[Memcached] file or folder: " << lpath.substr(1) << " is marked as existent in cache.");

        // get attributes from cache
        theCache->read_stat(stbuf,lpath.substr(1));
       }
       else 
       {
#endif
         bool haserror=false;
         unsigned int trycounter=0;
         S3ConnectionPtr lCon = getConnection();

         do{
           trycounter++;

           // get metadata from s3
           S3FS_TRY

             // check if we have that path without first /
             HeadResponsePtr lRes;
             lRes = lCon->head(theBucketname, lpath.substr(1));
             map_t lMap = lRes->getMetaData();
#ifndef NDEBUG
             S3_LOG(S3_DEBUG,"s3_getattr(...)","  requested metadata for " << lpath.substr(1));
             for (map_t::iterator lIter = lMap.begin(); lIter != lMap.end(); ++lIter) {
               S3_LOG(S3_DEBUG,"s3_getattr(...)","    got " << (*lIter).first << " : " << (*lIter).second);
             }
             S3_LOG(S3_DEBUG,"s3_getattr(...)","    content-length: " << lRes->getContentLength());
#endif

             // set the meta data in the stat struct
             fill_stat(lMap, stbuf, lRes->getContentLength());
           S3FS_CATCH(Head)
         }while(haserror && trycounter<AWS_TRIES_ON_ERROR);

         releaseConnection(lCon);
         lCon=NULL;

#ifdef S3FS_USE_MEMCACHED
         if(result==-ENOENT && !haserror){ 

           // remember in cache that file does not exist
           key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"").c_str();
           theCache->save_key(key, "0");
         }else if(result==0){

           //remember successfully retrieved data in cache
           theCache->save_key(key, "1");
           theCache->save_stat(stbuf, lpath.substr(1));
         }else{

           // on any other error invalidate the cache to force reload of data
           key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"").c_str();
           theCache->delete_key(key);
         }
#endif //USE_MEMCACHED

#ifdef S3FS_USE_MEMCACHED
       }
#endif

    }

  }catch(...){
    S3_LOG(S3_ERROR,location,"An Error occured while trying to get file attributes.");

#ifdef S3FS_USE_MEMCACHED

    // cleanup cache to prevent future errors
    std::string key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    return -EIO; // I/O Error
  }

  return result;
}


/*
 * Change the permission bits of a file
 */
static int
s3_chmod(const char * path, mode_t mode)
{
  S3_LOG(S3_DEBUG,"s3_chmod(...)","path: " << path << " mode: " << mode);

   // init result
   int result=0;

   //TODO 

   return result;

}

/*
 * Change the access and modification times of a file with nanosecond resolution
 */
static int
s3_utimens(const char *path, const struct timespec tv[2])
{
  if(tv){
    S3_LOG(S3_DEBUG,"s3_utimens(...)","path: " << path << " time:" << time_to_string(tv->tv_sec));
  }else{
    S3_LOG(S3_DEBUG,"s3_utimens(...)","path: " << path);
  }

  //init result
  int result=0;

  // TODO set new mtime

  return result;
}


/*
 * Change the owner and group of a file
 */
static int
s3_chown(const char * path, uid_t uid, gid_t gid)
{
  S3_LOG(S3_DEBUG,"s3_chown(...)","path: " << path << " uid:" << uid << " gid:" << gid);

  //init result
  int result=0;

  // TODO set new owner

  return result;

}


/*
 * Change the size of a file
 *
 */
static int 
s3_truncate(const char * path, off_t offset)
{
  S3_LOG(S3_DEBUG,"s3_truncate(...)","path: " << path << " offset:" << offset);

// initialize result
  int result=0;
  std::string lpath(path);
#ifdef S3FS_USE_MEMCACHED
  std::string key;
#endif // S3FS_USE_MEMCACHED
  std::auto_ptr<fuse_file_info> fileinfo(new fuse_file_info);

  try{
    if(offset==0){
      //get file stat
      struct stat stbuf;
      s3_getattr(path,&stbuf);

      std::auto_ptr<FileHandle> fileHandle(new FileHandle);

      // generate temp file and open it
      int ltempsize=theS3FSTempFolder.length();
      char ltempfile[ltempsize];
      strcpy(ltempfile,theS3FSTempFolder.c_str());
      fileHandle->id=mkstemp(ltempfile);
      fileHandle->filename = std::string(ltempfile);
      S3_LOG(S3_DEBUG,"s3_truncate(...)","File Descriptor # is: " << fileHandle->id << " file name = " << ltempfile);
      std::auto_ptr<std::fstream> tempfile(new std::fstream());
      tempfile->open(ltempfile, std::fstream::in | std::fstream::out | std::fstream::binary);

      // the file is empty. thats what we want.
      fileHandle->size=0;
      fileHandle->filestream = tempfile.release();
      fileHandle->is_write = true;
      fileHandle->mode = stbuf.st_mode;
      fileHandle->s3key = lpath.substr(1);
      fileHandle->mtime = getCurrentTime();

      //remember tempfile
      fileinfo->fh = (uint64_t)fileHandle->id;
      int lTmpPointer = fileHandle->id;
      tempfilemap.insert( std::pair<int,struct FileHandle*>(lTmpPointer,fileHandle.release()) );

#ifdef S3FS_USE_MEMCACHED

      // remember changes in cache
      stbuf.st_size=0;
      stbuf.st_mtime=getCurrentTime();
      theCache->save_stat(&stbuf,lpath.substr(1));

      // cleanup cache
      key=theCache->getkey(AWSCache::PREFIX_FILE,lpath.substr(1),"");
      theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

      // write the empty file to s3
      s3_release(path, fileinfo.get());

    }else{
      S3_LOG(S3_ERROR,"s3_truncate(...)","Truncate only implemented for 0.");
    }

    return result;
  }catch(...){
    S3_LOG(S3_ERROR,"s3_truncate(...)","An Error occured while trying to open a file.");

#ifdef S3FS_USE_MEMCACHED
    // cleanup cache to prevent future errors
    key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);
    key=theCache->getkey(AWSCache::PREFIX_FILE,lpath.substr(1),"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    return -EIO; // I/O Error
  }

  return result;

}

static int
s3_mkdir(const char *path, mode_t mode)
{
#ifndef NDEBUG
  std::string location="s3_mkdir";
#endif

  S3_LOG(S3_DEBUG,"s3_mkdir(...)","path: " << path << " mode: " << mode);

  int result=0;

  std::string lpath(path);

  S3ConnectionPtr lCon = getConnection();
  bool haserror=false;
  unsigned int trycounter=0;

  try{
    do{
      trycounter++;
      S3FS_TRY
        map_t lDirMap;
        lDirMap.insert(pair_t("dir", "1"));
        lDirMap.insert(pair_t("gid", to_string(getgid())));
        lDirMap.insert(pair_t("uid", to_string(getuid())));
        //lDirMap.insert(pair_t("mode", to_string(mode)));
        //TODO hack
        lDirMap.insert(pair_t("mode", "511"));
        lDirMap.insert(pair_t("mtime", time_to_string(getCurrentTime())));
        PutResponsePtr lRes = lCon->put(theBucketname, lpath.substr(1), 0, "text/plain", 0, &lDirMap);

        // success
#ifdef S3FS_USE_MEMCACHED
        // delete data from cache
        std::string key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
        theCache->delete_key(key);

        // delete cached dir entries of parent folder
        std::string parentfolder=AWSCache::getParentFolder(lpath.substr(1));
        key=theCache->getkey(AWSCache::PREFIX_DIR_LS,parentfolder,"");
        theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

        S3FS_EXIT(result);
      S3FS_CATCH(Put)
    }while(haserror && trycounter<AWS_TRIES_ON_ERROR);
  }catch(...){
    S3_LOG(S3_ERROR,"s3_mkdir","An Error occured while trying make dir.");

#ifdef S3FS_USE_MEMCACHED

    // cleanup cache to prevent future errors
    std::string key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);

    // delete cached dir entries of parent folder
    std::string parentfolder=AWSCache::getParentFolder(lpath.substr(1));
    key=theCache->getkey(AWSCache::PREFIX_DIR_LS,parentfolder,"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    S3FS_EXIT(-EIO); // I/O Error
  }

  S3FS_EXIT(result);
}



static int
s3_rmdir(const char *path)
{
#ifndef NDEBUG
  std::string location="s3_rmdir";
#endif

  S3_LOG(S3_DEBUG,"s3_rmdir(...)","path: " << path);

  int result=0;
  std::string lpath(path);
  bool haserror=false;
  unsigned int trycounter=0;
  S3ConnectionPtr lCon=NULL;

  try{
    // now we have to check if the folder is empty
#ifdef S3FS_USE_MEMCACHED
     std::string value;
     memcached_return rc;

     std::string key=theCache->getkey(AWSCache::PREFIX_DIR_LS,lpath.substr(1),"");

     value=theCache->read_key(key, &rc);
     if (rc==MEMCACHED_SUCCESS && value.length()>0) // there are entries in the folder
     {
       S3_LOG(S3_DEBUG,"s3_rmdir(...)","[Memcached] found entries for folder '" << lpath.substr(1) << "': " << value);
       return -ENOTEMPTY;
     }else if(rc==MEMCACHED_SUCCESS && value.length()==0){
       // folder empty -> can be removed
       S3_LOG(S3_DEBUG,"s3_rmdir(...)","[Memcached] folder '" << lpath.substr(1) << "' is empty.");
     }
     else 
     {
#endif // S3FS_USE_MEMCACHED

       lCon = getConnection();

       // we need a slash at the end because otherwise we would read files that start with the folder name,
       // but are not actually in the folder
       if(lpath.length()>0 && lpath.at(lpath.length()-1)!='/') lpath.append("/");

       std::string lentries="";

       do{
         trycounter++;
         ListBucketResponsePtr lRes;
         lentries="";
         S3FS_TRY
           std::string lMarker;
           do {
             // get object without first /
             S3_LOG(S3_DEBUG,"s3_rmdir(...)","list bucket: "<<theBucketname<<" prefix: "<<lpath.substr(1));
             lRes = lCon->listBucket(theBucketname, lpath.substr(1), lMarker, "/", -1);
             lRes->open();
             ListBucketResponse::Object o;
             while (lRes->next(o)) {
               struct stat lStat;
               memset(&lStat, 0, sizeof(struct stat));
               S3_LOG(S3_DEBUG,"s3_rmdir(...)","result: " << o.KeyValue);
               std::string lTmp = o.KeyValue.replace(0, lpath.length()-1, "");

#ifdef S3FS_USE_MEMCACHED
               // remember entries
               if(lentries.length()>0) {
                 lentries.append(AWSCache::DELIMITER_FOLDER_ENTRIES);
               }
#endif
               lentries.append(lTmp);

               lMarker = o.KeyValue;
             }
             lRes->close();
           } while (lRes->isTruncated());

         S3FS_CATCH(ListBucket);
       }while(haserror && trycounter<AWS_TRIES_ON_ERROR);

       if(result==0 && lentries.length()>0){ 

         // folder not empty
         result=-ENOTEMPTY;

#ifdef S3FS_USE_MEMCACHED

         //remember successfully retrieved entries in cache
         key=theCache->getkey(AWSCache::PREFIX_DIR_LS,lpath.substr(1),"");
         theCache->save_key(key, lentries);
#endif // S3FS_USE_MEMCACHED

         S3FS_EXIT(result);
       }

#ifdef S3FS_USE_MEMCACHED
    }
#endif // S3FS_USE_MEMCACHED

    // folder is empty -> can be deleted
    if(lpath.length()>0 && lpath.at(lpath.length()-1)=='/') {
      lpath=lpath.substr(0,lpath.length()-1);
    }
    haserror=false;
    trycounter=0;
    if(lCon==NULL) lCon = getConnection();

    // delete folder on s3
    do{
      trycounter++;
      S3FS_TRY
        DeleteResponsePtr lRes = lCon->del(theBucketname, lpath.substr(1));
      S3FS_CATCH(Put)
    }while(haserror && trycounter<AWS_TRIES_ON_ERROR);

#ifdef S3FS_USE_MEMCACHED
    if(result==0){ // successfully deleted

      // remember in cache that folder does not exist any more
      key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
      theCache->save_key(key,"0");
    }else{

      // delete key to force reload to prevent future errors
      key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
      theCache->delete_key(key);
    }

    // delete cached folder entries
    key=theCache->getkey(AWSCache::PREFIX_DIR_LS,lpath.substr(1),"");
    theCache->delete_key(key);

    // delete cached dir entries of parent folder
    std::string parentfolder=AWSCache::getParentFolder(lpath.substr(1));
    key=theCache->getkey(AWSCache::PREFIX_DIR_LS,parentfolder,"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    S3FS_EXIT(result);
  }catch(...){
    S3_LOG(S3_ERROR,"s3_rmdir","An Error occured while trying to remove dir.");

#ifdef S3FS_USE_MEMCACHED

    // cleanup cache to prevent future errors
    std::string key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);

    // delete cached folder entries
    key=theCache->getkey(AWSCache::PREFIX_DIR_LS,lpath.substr(1),"");
    theCache->delete_key(key);

    // delete cached dir entries of parent folder
    std::string parentfolder=AWSCache::getParentFolder(lpath.substr(1));
    key=theCache->getkey(AWSCache::PREFIX_DIR_LS,parentfolder,"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    if(lCon) releaseConnection(lCon);
    lCon=NULL;
    return -EIO; // I/O Error
  }

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
  std::string location="s3_readdir";
#endif

  S3_LOG(S3_DEBUG,"s3_readdir(...)","readdir: " << path);

  int result=0;
  S3ConnectionPtr lCon = NULL;

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);

  // TODO handle full buffer by remembering the marker

  std::string lpath(path);
  if (lpath.at(lpath.length()-1) != '/')
    lpath += "/";

  try{
#ifdef S3FS_USE_MEMCACHED
    std::string value;
    memcached_return rc;

    std::string key=theCache->getkey(AWSCache::PREFIX_DIR_LS,lpath.substr(1),"");

    value=theCache->read_key(key, &rc);
    if (rc==MEMCACHED_SUCCESS) // there are entries in the cache for this folder
    {
      S3_LOG(S3_DEBUG,"s3_readdir(...)","[Memcached] found entries for folder '" << lpath.substr(1) << "': " << value);
      std::vector<std::string> items;
      std::vector<std::string>::iterator iter;

      AWSCache::to_vector(items,value,AWSCache::DELIMITER_FOLDER_ENTRIES);
      iter=items.begin();
      while(iter!=items.end()){
        struct stat lStat;
        memset(&lStat, 0, sizeof(struct stat));

        filler(buf, (*iter).c_str(), &lStat, 0);

        iter++;
      }
    }
    else 
    {
      std::string lentries="";
#endif

      lCon = getConnection();
      bool haserror=false;
      unsigned int trycounter=0;

      do{
        trycounter++;
        ListBucketResponsePtr lRes;
        S3FS_TRY
          std::string lMarker;
          do {
            // get object without first /
            S3_LOG(S3_DEBUG,"s3_readdir(...)","list bucket: "<<theBucketname<<" prefix: "<<lpath.substr(1));
            lRes = lCon->listBucket(theBucketname, lpath.substr(1), lMarker, "/", -1);
            lRes->open();
            ListBucketResponse::Object o;
            while (lRes->next(o)) {
              struct stat lStat;
              memset(&lStat, 0, sizeof(struct stat));

              S3_LOG(S3_DEBUG,"s3_readdir(...)","  result: " << o.KeyValue);
              std::string lTmp = o.KeyValue.replace(0, lpath.length()-1, "");

#ifdef S3FS_USE_MEMCACHED
              // remember entries to store in cache
              if(lentries.length()>0) lentries.append(AWSCache::DELIMITER_FOLDER_ENTRIES);
              lentries.append(lTmp);
#endif //S3FS_USE_MEMCACHED

              filler(buf, lTmp.c_str(), &lStat, 0);
              lMarker = o.KeyValue;
            }
            lRes->close();
          } while (lRes->isTruncated());

         S3FS_CATCH(ListBucket);
       }while(haserror && trycounter<AWS_TRIES_ON_ERROR);

#ifdef S3FS_USE_MEMCACHED
       if(result==-ENOENT && !haserror){ 

         // remember in cache that no entries exist in folder
         theCache->save_key(key, "");
       }else if (!haserror){

         //remember successfully retrieved entries in cache
         theCache->save_key(key, lentries);
       }
#endif

       S3FS_EXIT(result);

#ifdef S3FS_USE_MEMCACHED
    }
#endif
  }catch(...){
    S3_LOG(S3_ERROR,"s3_readdir","An Error occured while trying to read dir contents.");

#ifdef S3FS_USE_MEMCACHED

    // cleanup cache to prevent future errors
    std::string key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);

    // delete cached folder entries
    key=theCache->getkey(AWSCache::PREFIX_DIR_LS,lpath.substr(1),"");
    theCache->delete_key(key);

    // delete cached dir entries of parent folder
    std::string parentfolder=AWSCache::getParentFolder(lpath.substr(1));
    key=theCache->getkey(AWSCache::PREFIX_DIR_LS,parentfolder,"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    if(lCon) releaseConnection(lCon);
    lCon=NULL;
    return -EIO; // I/O Error
  }
  return result;
}


/*
 * Create and open a file
 * 
 * If the file does not exist, first create it with the specified 
 * mode, and then open it.
 * 
 * If this method is not implemented or under Linux kernel versions 
 * earlier than 2.6.15, the mknod() and open() methods will be called 
 * instead.
 * 
 */
static int
s3_create(const char *path, mode_t mode, struct fuse_file_info *fileinfo)
{
  S3_LOG(S3_DEBUG,"s3_create(...)","path: " << path << " mode: " << mode);

  std::string lpath(path);
  std::auto_ptr<FileHandle> fileHandle(new FileHandle);

  // initialize result
  int result=0;
  memset(fileinfo, 0, sizeof(struct fuse_file_info));

  try{

    // generate temp file and open it
    int ltempsize=theS3FSTempFolder.length();
    char ltempfile[ltempsize];
    strcpy(ltempfile,theS3FSTempFolder.c_str());
    fileHandle->id=mkstemp(ltempfile);
    S3_LOG(S3_DEBUG,"s3_create(...)","File Descriptor # is: " << fileHandle->id << " file name = " << ltempfile);
    std::auto_ptr<std::fstream> tempfile(new std::fstream());
    tempfile->open(ltempfile);

    fileHandle->filename = std::string(ltempfile);
    fileHandle->size = 0;
    fileHandle->s3key = lpath.substr(1);// cut off the first slash
    //TODO this is a hack
    fileHandle->mode = S_IFREG | 0777;
    fileHandle->filestream = tempfile.release();
    fileHandle->is_write = true;
    fileHandle->mtime = getCurrentTime();

    //remember filehandle
    fileinfo->fh = (uint64_t)fileHandle->id;
    int lTmpPointer = fileHandle->id;
    tempfilemap.insert( std::pair<int,struct FileHandle*>(lTmpPointer,fileHandle.release()) );

#ifdef S3FS_USE_MEMCACHED

    // init stat
    struct stat stbuf;
    //TODO this is a hack
    stbuf.st_mode = S_IFREG | 0777;
    stbuf.st_gid = getgid();
    stbuf.st_uid = getuid();
    stbuf.st_mtime = getCurrentTime();
    stbuf.st_size = 0;
    stbuf.st_nlink = 1;

    // store data for newly created file to cache
    std::string key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"").c_str();
    theCache->save_key(key, "1");
    theCache->save_stat(&stbuf, lpath.substr(1));

    // delete cached dir entries of parent folder
    std::string parentfolder=AWSCache::getParentFolder(lpath.substr(1));
    key=theCache->getkey(AWSCache::PREFIX_DIR_LS,parentfolder,"").c_str();
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED
  }catch(...){
    S3_LOG(S3_ERROR,"s3_create","An Error occured while trying to create a new file.");

#ifdef S3FS_USE_MEMCACHED

    // cleanup cache to prevent future errors
    std::string key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);

    // delete cached dir entries of parent folder
    std::string parentfolder=AWSCache::getParentFolder(lpath.substr(1));
    key=theCache->getkey(AWSCache::PREFIX_DIR_LS,parentfolder,"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    return -EIO; // I/O Error
  }
  return result;
}


/*
 * Remove a file 
 *
 */
static int
s3_unlink(const char * path)
{
#ifndef NDEBUG
  std::string location="s3_unlink";
#endif

  S3_LOG(S3_DEBUG,"s3_unlink(...)","path: " << path);

  S3ConnectionPtr lCon = NULL;
  int result=0;
  std::string lpath(path);
#ifdef S3FS_USE_MEMCACHED
  std::string key;
#endif // S3FS_USE_MEMCACHED

  try{
    lCon = getConnection();

    bool haserror=false;
    unsigned int trycounter=0;

    do{
      trycounter++;
      S3FS_TRY
        DeleteResponsePtr lRes = lCon->del(theBucketname, lpath.substr(1));
      S3FS_CATCH(Put)
    }while(haserror && trycounter<AWS_TRIES_ON_ERROR);

#ifdef S3FS_USE_MEMCACHED
    if(result!=-ENOENT){
      // delete data from cache
      key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
      theCache->delete_key(key);

      // delete cached dir entries of parent folder
      std::string parentfolder=AWSCache::getParentFolder(lpath.substr(1));
      key=theCache->getkey(AWSCache::PREFIX_DIR_LS,parentfolder,"");
      theCache->delete_key(key);
    }
#endif // S3FS_USE_MEMCACHED

    S3FS_EXIT(result);
  }catch(...){
    S3_LOG(S3_ERROR,"s3_unlink(...)","An Error occured while trying to delete a file.");

#ifdef S3FS_USE_MEMCACHED
    // cleanup cache to prevent future errors
    std::string key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);

    // delete cached dir entries of parent folder
    std::string parentfolder=AWSCache::getParentFolder(lpath.substr(1));
    key=theCache->getkey(AWSCache::PREFIX_DIR_LS,parentfolder,"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

     if(lCon) releaseConnection(lCon);
     lCon=NULL;
     return -EIO; // I/O Error
  }
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
  std::string location="s3_open";
#endif

  S3_LOG(S3_DEBUG,"s3_open(...)","path: " << path);

  // initialize
  int result=0;
  std::string lpath(path);
  S3ConnectionPtr lCon = NULL;
#ifdef S3FS_USE_MEMCACHED
  std::string key;
#endif // S3FS_USE_MEMCACHED

  try{
    //get file stat
    struct stat stbuf;
    s3_getattr(path,&stbuf);

    std::auto_ptr<FileHandle> fileHandle(new FileHandle);

    memset(fileinfo, 0, sizeof(struct fuse_file_info));

    // generate temp file and open it
    int ltempsize=theS3FSTempFolder.length();
    char ltempfile[ltempsize];
    strcpy(ltempfile,theS3FSTempFolder.c_str());
    fileHandle->id=mkstemp(ltempfile);
    fileHandle->filename = std::string(ltempfile);
    S3_LOG(S3_DEBUG,"s3_open(...)","File Descriptor # is: " << fileHandle->id << " file name = " << ltempfile);
    std::auto_ptr<std::fstream> tempfile(new std::fstream());
    tempfile->open(ltempfile, std::fstream::in | std::fstream::out | std::fstream::binary);

#ifdef S3FS_USE_MEMCACHED
    //init
    bool got_file_cont_from_cache=false;
    memcached_return rc;
    unsigned int filesize=(unsigned int)stbuf.st_size;

    // file can only be in cach if content is not too big
    if(filesize<AWSCache::FILE_CACHING_UPPER_LIMIT){
      key=theCache->getkey(AWSCache::PREFIX_FILE,lpath.substr(1),"").c_str();
      theCache->read_file(key,dynamic_cast<std::fstream*>(tempfile.get()),&rc);

      if (rc==MEMCACHED_SUCCESS){
        got_file_cont_from_cache=true;
        fileHandle->size=stbuf.st_size;
        fileHandle->filestream = tempfile.release();
        fileHandle->is_write = false;
        fileHandle->mode = stbuf.st_mode;
        fileHandle->s3key = lpath.substr(1);

        //remember tempfile
        fileinfo->fh = (uint64_t)fileHandle->id;
        int lTmpPointer = fileHandle->id;
        tempfilemap.insert( std::pair<int,struct FileHandle*>(lTmpPointer,fileHandle.release()) );
      }
    }

    if(!got_file_cont_from_cache){
#endif // S3FS_USE_MEMCACHED

      // now lets get the data and save it into the temp file
      lCon = getConnection();
      bool haserror=false;
      unsigned int trycounter=0;

      do{
        trycounter++;
        S3FS_TRY
          GetResponsePtr lGet = lCon->get(theBucketname, lpath.substr(1));
          std::istream& lInStream = lGet->getInputStream();
          S3_LOG(S3_DEBUG,"s3_open(...)","content length: " << lGet->getContentLength());
          fileHandle->size=lGet->getContentLength();

          // write data to temp file
          while (lInStream.good())     // loop while extraction from file is possible
          {
            char data;
            lInStream.get(data);       // get character from file
            tempfile->put(data);
          }
          tempfile->flush();

          fileHandle->filestream = tempfile.release();
          fileHandle->is_write = false;
          fileHandle->mtime = getCurrentTime();
          fileHandle->mode = stbuf.st_mode;
          fileHandle->s3key = lpath.substr(1);

          //remember tempfile
          fileinfo->fh = (uint64_t)fileHandle->id;
          int lTmpPointer = fileHandle->id;
          tempfilemap.insert( std::pair<int,struct FileHandle*>(lTmpPointer,fileHandle.release()) );

        S3FS_CATCH(Get)
      }while(haserror && trycounter<AWS_TRIES_ON_ERROR);

#ifdef S3FS_USE_MEMCACHED
    }
#endif // S3FS_USE_MEMCACHED
    if (result!=0){
      fileinfo->fh = NULL;
    }
    return result;
  }catch(...){
    S3_LOG(S3_ERROR,"s3_open(...)","An Error occured while trying to open a file.");

#ifdef S3FS_USE_MEMCACHED
    // cleanup cache to prevent future errors
    key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);
    key=theCache->getkey(AWSCache::PREFIX_FILE,lpath.substr(1),"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

     if(lCon) releaseConnection(lCon);
     lCon=NULL;
     return -EIO; // I/O Error
  }
}


/*
 * Write data to an open file
 * 
 * Write should return exactly the number of bytes requested except 
 * on error. An exception to this is when the 'direct_io' mount option 
 * is specified (see read operation).
 * 
 * 
 */
static int
s3_write(const char * path, const char * data, size_t size, off_t offset, struct fuse_file_info * fileinfo)
{
  S3_LOG(S3_DEBUG,"s3_write(...)","path: " << path << " data: " << data << " size: " << size << " offset: " << offset);

  S3_LOG(S3_DEBUG,"s3_write(...)","data size: " << strlen(data));

  // init result
  int result=0;
  std::string lpath(path);
#ifdef S3FS_USE_MEMCACHED
  std::string key;
#endif // S3FS_USE_MEMCACHED

  try{
    if( 
       (((int)fileinfo->fh)!=0) && 
       (tempfilemap.find((int)fileinfo->fh)!=tempfilemap.end())
      ){
      FileHandle* fileHandle=tempfilemap.find((int)fileinfo->fh)->second;
      std::string tempfilename=fileHandle->filename;
      std::fstream* tempfile=fileHandle->filestream;

      // write data to temp file
      tempfile->seekp(offset,std::ios_base::beg);
      tempfile->write(data,size);

      // flag to update file on s3
      fileHandle->is_write = true;

      result=size;
    }else{
      S3_LOG(S3_ERROR,"s3_write(...)","No temporary file handle exists.");
      return -EIO;
    }
    return result;
  }catch(...){
    S3_LOG(S3_ERROR,"s3_write(...)","An Error occured while trying write data to a file.");

#ifdef S3FS_USE_MEMCACHED
    // cleanup cache to prevent future errors
    key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);
    key=theCache->getkey(AWSCache::PREFIX_FILE,lpath.substr(1),"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    return -EIO; // I/O Error
  }
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
  std::string location="s3_release";
#endif

  S3_LOG(S3_DEBUG,"s3_release(...)","path: " << path);

  // initialize result
  int result=0;
  std::string lpath(path);
  S3ConnectionPtr lCon = NULL;
#ifdef S3FS_USE_MEMCACHED
  std::string key;
#endif // S3FS_USE_MEMCACHED

  try{
    if(fileinfo!=NULL
        && (int)fileinfo->fh!=0){

      // get filehandle struct
      std::map<int,struct FileHandle*>::iterator foundtempfile=tempfilemap.find((int)fileinfo->fh);
      if(foundtempfile!=tempfilemap.end()){
         std::auto_ptr<FileHandle> fileHandle(foundtempfile->second);

        // check if we have to send changes to s3
        if(fileHandle->is_write){

          // reset filestream
          fileHandle->filestream->seekg(0,std::ios_base::beg);

          // transfer temp file to s3
          lCon = getConnection();
          bool haserror=false;
          unsigned int trycounter=0;

          do{
            trycounter++;
            S3FS_TRY
              map_t lDirMap;
              lDirMap.insert(pair_t("file", "1"));
              lDirMap.insert(pair_t("gid", to_string(getgid())));
              lDirMap.insert(pair_t("uid", to_string(getuid())));
              lDirMap.insert(pair_t("mode", to_string(fileHandle->mode)));
              lDirMap.insert(pair_t("mtime", time_to_string(fileHandle->mtime)));
              PutResponsePtr lRes = lCon->put(theBucketname, fileHandle->s3key, *(fileHandle->filestream), "text/plain", &lDirMap);

#ifdef S3FS_USE_MEMCACHED
              // invalidate cached data of file
              key=theCache->getkey(AWSCache::PREFIX_FILE,lpath.substr(1),"").c_str();
              theCache->save_file(key,dynamic_cast<std::fstream*>(fileHandle->filestream),fileHandle->size); 
              key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"").c_str();
              theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

            S3FS_CATCH(Put)
          }while(haserror && trycounter<AWS_TRIES_ON_ERROR);

          if(result!=0){ 
            S3_LOG(S3_ERROR,"s3_release(...)","saving file on s3 failed");
          }

        }else{ 
          // we have to send no changes to s3 -> readonly

#ifdef S3FS_USE_MEMCACHED
          // only cache file content if not too big
          if(fileHandle->size < AWSCache::FILE_CACHING_UPPER_LIMIT){
              key=theCache->getkey(AWSCache::PREFIX_FILE,lpath.substr(1),"").c_str();
              theCache->save_file(key,dynamic_cast<std::fstream*>(fileHandle->filestream),fileHandle->size); 
           }
#endif // S3FS_USE_MEMCACHED
        }

      }else{
        S3_LOG(S3_INFO,"s3_release(...)","couldn't find filehandle.");
      }
    }else{
      S3_LOG(S3_INFO,"s3_release(...)","no fileinfo or filehandle-ID provided.");
    }
    return result;
  }catch(...){
    S3_LOG(S3_ERROR,"s3_release(...)","An Error occured while trying to release a file.");

#ifdef S3FS_USE_MEMCACHED
    // cleanup cache to prevent future errors
    key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);
    key=theCache->getkey(AWSCache::PREFIX_FILE,lpath.substr(1),"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    if(lCon) releaseConnection(lCon);
    lCon=NULL;
    return -EIO; // I/O Error
  }
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
  S3_LOG(S3_DEBUG,"s3_read(...)","path: " << path << " offset: " << offset << " size: " << size);
  S3_LOG(S3_DEBUG,"s3_read(...)","size of the tempfilemap " << tempfilemap.size());

  std::string lpath(path);
#ifdef S3FS_USE_MEMCACHED
  std::string key;
#endif // S3FS_USE_MEMCACHED

  try{
    FileHandle* fileHandle=tempfilemap.find((int)fileinfo->fh)->second;
    std::string tempfilename=fileHandle->filename;
    std::fstream* tempfile=fileHandle->filestream;

    // get length of file:
    unsigned int filelength = (unsigned int) fileHandle->size;

    int readsize = 0;
    if(size>(filelength-offset)){
      readsize=filelength-offset;
    }else{
      readsize=size;
    }

    tempfile->seekg(offset,std::ios_base::beg);
    memset(buf, 0, readsize); 
    tempfile->read(buf,readsize);
    S3_LOG(S3_DEBUG,"s3_read(...)","readsize: " << readsize << " tempfile->gcount(): " << tempfile->gcount());
    assert(readsize == tempfile->gcount());
    return readsize;
  }catch(...){
    S3_LOG(S3_ERROR,"s3_read(...)","An Error occured while trying to read a file.");

#ifdef S3FS_USE_MEMCACHED
    // cleanup cache to prevent future errors
    key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);
    key=theCache->getkey(AWSCache::PREFIX_FILE,lpath.substr(1),"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    return -EIO; // I/O Error
  }

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
  S3_LOG(S3_DEBUG,"s3_opendir(...)","path: " << path);

  // by default allow access
  return 0;
}



static struct fuse_operations s3_filesystem_operations;


int
main(int argc, char **argv)
{
  // set callback functions
  s3_filesystem_operations.getattr    = s3_getattr;
  s3_filesystem_operations.utimens = s3_utimens;
  s3_filesystem_operations.truncate = s3_truncate;
  s3_filesystem_operations.chmod = s3_chmod;
  s3_filesystem_operations.chown = s3_chown;
  s3_filesystem_operations.mkdir      = s3_mkdir;
  s3_filesystem_operations.rmdir      = s3_rmdir;
  s3_filesystem_operations.readdir    = s3_readdir;
  s3_filesystem_operations.create     = s3_create;
  s3_filesystem_operations.unlink     = s3_unlink;
  s3_filesystem_operations.opendir     = s3_opendir;
  // can't be supported because s3 doesn't allow to change meta data without putting the object again
  s3_filesystem_operations.read       = s3_read;
  s3_filesystem_operations.write       = s3_write;
  s3_filesystem_operations.open       = s3_open;
  s3_filesystem_operations.release       = s3_release;

  // initialization
  theFactory = AWSConnectionFactory::getInstance();

  // parse args
  std::vector<std::string> argv_fuse_vector;
  //std::cerr << "argc: "<< argc << " argv: " << *argv << std::endl;

  for(int argindex=0;argindex<argc;argindex++){
     std::string arg(argv[argindex]);
     if(arg.compare("--bucket")==0) {
        if(argv[argindex+1][0]=='-') {
           std::cerr << "Bucketname missing after '--bucket'" << std::endl;
           return 5;
        } 
        else {
           theBucketname = std::string(argv[argindex+1]);
           ++argindex;
        }
     } 
     else if(arg.compare("--property-file")==0) {
        if(argv[argindex+1][0]=='-') {
           std::cerr << "Property file missing after '--propery-file'" << std::endl;
           return 5;
        }
        else {
           thePropertyFile = std::string(argv[argindex+1]);
           ++argindex;
        }
     }
     else {
        // standardbehaviour: dont know the arg, so pass it on to fuse
        argv_fuse_vector.push_back(arg);
     }
  }

  if (thePropertyFile != "") {
    s3fs::utils::PropertyUtil::PropertyMapT lProperties;
    s3fs::utils::PropertyUtil::read(thePropertyFile.c_str(), lProperties);       
    if (theBucketname == "")
      theBucketname = lProperties[s3fs::utils::Properties::BUCKET_NAME];
    theAccessKeyId = lProperties[s3fs::utils::Properties::AWS_ACCESS_KEY];
    theSecretAccessKey = lProperties[s3fs::utils::Properties::AWS_SECRET_ACCESS_KEY];
  } else {
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
  }

  if (theAccessKeyId.length() == 0) {
    std::cerr << "Please specify the AWS_ACCESS_KEY environment variable" << std::endl;
    return 1;
  }
  if (theSecretAccessKey.length() == 0) {
    std::cerr << "Please specify the AWS_SECRET_ACCESS_KEY environment variable" << std::endl;
    return 2;
  }


  char** argv_fuse=new char*[ argv_fuse_vector.size()];
  for (unsigned int i=0;i<argv_fuse_vector.size();i++ )
  {
     argv_fuse[i]= const_cast<char*>(argv_fuse_vector[i].c_str());
  }

  if(theBucketname.length()==0) {
    std::cerr << "Please provide a bucketname that you would like to mount (use --bucket option)." << std::endl;
    return 6; 
  }
#ifdef S3FS_USE_MEMCACHED
  else
  {
    theCache = new AWSCache(theBucketname);
  }
#endif //S3FS_USE_MEMCACHED

  if(getenv("S3FS_TEMP")!=NULL){
    theS3FSTempFolder = getenv("S3FS_TEMP");
    if(theS3FSTempFolder.length()>0 && theS3FSTempFolder.at(theS3FSTempFolder.length()-1)=='/'){
      theS3FSTempFolder.append("s3fs_file_XXXXXX");
    }else{
      theS3FSTempFolder.append("/s3fs_file_XXXXXX");
    }
  }else if(getenv("TMP")!=NULL){
    theS3FSTempFolder = getenv("TMP");
    if(theS3FSTempFolder.length()>0 && theS3FSTempFolder.at(theS3FSTempFolder.length()-1)=='/'){
      theS3FSTempFolder.append("s3fs_file_XXXXXX");
    }else{
      theS3FSTempFolder.append("/s3fs_file_XXXXXX");
    }
  }else if(getenv("TMPDIR")!=NULL){
    theS3FSTempFolder = getenv("TMPDIR");
    if(theS3FSTempFolder.length()>0 && theS3FSTempFolder.at(theS3FSTempFolder.length()-1)=='/'){
      theS3FSTempFolder.append("s3fs_file_XXXXXX");
    }else{
      theS3FSTempFolder.append("/s3fs_file_XXXXXX");
    }
  }else{
    std::cerr << "Please specify the S3FS_TEMP environment variable. It should point to a folder where you have write access." << std::endl;
    return 3;
  }


  theS3ConnectionPool = new ConnectionPool<S3ConnectionPtr>(CONNECTION_POOL_SIZE,theAccessKeyId,theSecretAccessKey);

  std::cerr << "####################" << std::endl;
  std::cerr << "####################" << std::endl;
  std::cerr << "# S3FS starting up #" << std::endl;
  std::cerr << "####################" << std::endl;
  std::cerr << "####################" << std::endl;
  std::cerr << "Mounted Bucket is >>" << theBucketname << "<<" << std::endl;

  int lRes = fuse_main(argv_fuse_vector.size(), argv_fuse, &s3_filesystem_operations, NULL);

  delete[] argv_fuse;
#ifdef S3FS_USE_MEMCACHED
  delete theCache;
#endif //S3FS_USE_MEMCACHED
  delete theS3ConnectionPool;
  theFactory->shutdown();

  return lRes;
}

