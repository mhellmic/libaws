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
#include <stdio.h>
#include <unistd.h>

#include <libaws/aws.h>
#include "properties.h"

#ifdef S3FS_USE_MEMCACHED
#  include <libmemcached/memcached.h>
#  include "awscache.h"
#endif //USE_MEMCACHED

using namespace aws;

#ifdef S3FS_USE_MEMCACHED
std::auto_ptr<AWSCache> theCache;
#endif //USE_MEMCACHED

AWSConnectionFactory* theFactory;
std::auto_ptr<ConnectionPool<S3ConnectionPtr> > theS3ConnectionPool;
static unsigned int CONNECTION_POOL_SIZE=5;
static unsigned int AWS_TRIES_ON_ERROR=3;

std::string theAccessKeyId;
std::string theSecretAccessKey;
std::string theS3FSTempFolder;
std::string theS3FSTempFilePattern;
std::string theBucketname;
std::string thePropertyFile;
std::string theMemcachedServers;

static std::string DELIMITER_FOLDER_ENTRIES=",";

//And a more complex example:
struct s3fs_config {
  char* access_key;
  char* secret_key;
  char* temp_dir;
  char* property_file;
  char* bucket;
  char* memcached_servers;
  int   log_level;
  int   create_mount_dir;
};

enum {
  KEY_HELP,
};

#define S3FS_OPT(t, p, v) { t, offsetof(struct s3fs_config, p), v }

static struct fuse_opt s3fs_opts[] = {
   S3FS_OPT("config-file=%s",       property_file, 0),
   S3FS_OPT("access-key=%s",        access_key, 0),
   S3FS_OPT("secret-key=%s",        secret_key, 0),
   S3FS_OPT("temp-dir=%s",          temp_dir, 0),
   S3FS_OPT("bucket=%s",            bucket, 0),
   S3FS_OPT("log-level=%i",         log_level, 0),
   S3FS_OPT("memcached-servers=%s", memcached_servers, 0),
   S3FS_OPT("create-mountdir=%i", create_mount_dir, 0),

   FUSE_OPT_KEY("-h",             KEY_HELP),
   FUSE_OPT_KEY("-H",             KEY_HELP),
   FUSE_OPT_KEY("--help",         KEY_HELP),

   {NULL, -1U, 0} // mark the end
};
#undef S3FS_OPT

static std::map<int,struct FileHandle*> tempfilemap;
static struct fuse_operations s3_filesystem_operations;

static int s3fs_opt_proc(void *data, const char *arg, int key, struct fuse_args *outargs)
{
  switch (key) {
  case KEY_HELP:
    fprintf(stderr,
            "usage: %s mountpoint [options]\n"
            "\n"
            "general options:\n"
            "    -o opt,[opt...]       mount options\n"
            "    -h   -H   --help      print help\n"
            "\n"
            "S3FS options:\n"
            "    -o config-file=STRING       config file that may contain the options below\n"
            "    -o access-key=STRING        AWS Access Key ID\n"
            "    -o secret-key=STRING        AWS Secret Access Key\n"
            "    -o temp-dir=STRING          temporary directory used by s3fs\n"
            "    -o bucket=STRING            bucket to mount\n"
            "    -o memcached_servers=STRING memcached servers used for caching\n"
            "    -o log-level=INT            logging level (0=ERROR, 1=INFO, 2=DEBUG)\n"
            "    -o create-mountdir=INT      create mount dir if not existent? (0=no, 1=yes)\n"
            , outargs->argv[0]);
    fuse_opt_add_arg(outargs, "-ho");
    fuse_main(outargs->argc, outargs->argv, &s3_filesystem_operations, NULL);
    exit(1);
  }
  return 1;
}

enum LogLevel {
  S3_ERROR = 0,
  S3_INFO  = 1,
  S3_DEBUG = 2
};

static LogLevel theLogLevel = S3_ERROR;

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
 * checkTempFolder()
 *
 * checks if the theS3FSTempFolder exists. if it doesn't exist for whatever reason, it is made.
 */
static void checkTempFolder(){
  struct stat st;
  if (stat(theS3FSTempFolder.c_str(), &st) == -1) {
    std::cerr << "temporary directory does not exist " << theS3FSTempFolder << std::endl;
    if (::mkdir(theS3FSTempFolder.c_str() ,0777) == -1) {
      std::cerr << "couldnt create temporary directory"  << std::endl;
    } else {
      std::cerr << "created directory " << theS3FSTempFolder << std::endl;
    }
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
 * logging macros
 */
#ifdef S3FS_LOG_SYSLOG
#  define S3_LOG_OUTPUT(level, log_message) \
     syslog( level, "%s", log_message );
#else
#  define S3_LOG_OUTPUT(level, log_message) \
     std::cerr << log_message << std::endl;
#endif
    

#define S3_LOG_DEBUG(message) \
   do { \
     if (theLogLevel <= S3_DEBUG) { \
       std::ostringstream logMessage; \
       logMessage << "(func: " << __FUNCTION__ << "; line: " << __LINE__ << ") " \
                  << "[DEBUG] ## " << message << " ## "; \
       S3_LOG_OUTPUT(LOG_DEBUG, logMessage.str().c_str()); \
     } \
   } while (0);

#define S3_LOG_INFO(message) \
   do { \
     if (theLogLevel <= S3_INFO) { \
       std::ostringstream logMessage; \
       logMessage << "(func: " << __FUNCTION__ << "; line: " << __LINE__ << ") " \
                  << "[INFO] ## " << message << " ## "; \
       S3_LOG_OUTPUT(LOG_INFO, logMessage.str().c_str()); \
     } \
   } while (0);

#define S3_LOG_ERROR(message) \
   do { \
     if (theLogLevel <= S3_ERROR) { \
       std::ostringstream logMessage; \
       logMessage << "(func: " << __FUNCTION__ << "; line: " << __LINE__ << ") " \
                  << "[ERROR] ## " << message << " ## "; \
       S3_LOG_OUTPUT(LOG_ERR, logMessage.str().c_str()); \
     } \
   } while (0);

/**
 * macros that should be used for try-catch combinations
 */
#define S3FS_TRY try {

#ifndef NDEBUG
#  define S3FS_CATCH(kind) \
    } catch (kind ## Exception & s3Exception) { \
      S3_LOG_ERROR("S3Exception(ERRORCODE="<<((int)s3Exception.getErrorCode())<<"):"<<s3Exception.what()); \
      if (s3Exception.getErrorCode() != aws::S3Exception::NoSuchKey) { \
         haserror=true; \
         result=-EIO;\
      } else{ \
         haserror=false; \
         result=-ENOENT;\
      } \
    } catch (AWSConnectionException & conException) { \
     S3_LOG_ERROR("AWSConnectionException: "<<conException.what()); \
      haserror=true; \
      result=-ECONNREFUSED; \
    }catch (AWSException & awsException) { \
      S3_LOG_ERROR("AWSException: "<<awsException.what()); \
      haserror=true; \
      result=-EIO;\
    }
#else
#  define S3FS_CATCH(kind) \
    } catch (kind ## Exception & s3Exception) { \
      if (s3Exception.getErrorCode() != aws::S3Exception::NoSuchKey) { \
         haserror=true; \
         result=-EIO;\
      } else{ \
         haserror=false; \
         result=-ENOENT;\
      } \
    } catch (AWSConnectionException & conException) { \
      haserror=true; \
      result=-ECONNREFUSED; \
    }catch (AWSException & awsException) { \
      haserror=true; \
      result=-EIO;\
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

      S3_LOG_DEBUG("requested getattr for root / => exit");
      return result;
    } else if (strcmp(path, "/s3fs.stat") == 0) {
      stbuf->st_mode |= S_IFREG;
      stbuf->st_size = 0;
      stbuf->st_nlink = 1;
      stbuf->st_gid  = getgid();
      stbuf->st_uid  = getuid();
      stbuf->st_mtime  = getCurrentTime();

      S3_LOG_DEBUG("requested getattr for s3fs.stat => exit");
      return result;
    } else {

#ifdef S3FS_USE_MEMCACHED
      std::string value;

      memcached_return rc;

      // check if the cache knows if the file/folder exists
      std::string key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
      value=theCache->read_key(key, &rc);
      if (value.length() > 0 && value.compare("0")==0) // file does not exist
      {
        S3_LOG_DEBUG("[Memcached] file or folder: " << lpath.substr(1) << " is marked as non existent in cache.");
        return -ENOENT;
      }else if(value.length() > 0 && value.compare("1")==0) // file does exist
      {
        S3_LOG_DEBUG("[Memcached] file or folder: " << lpath.substr(1) << " is marked as existent in cache.");

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
					 if(haserror){	
					 	  S3_LOG_INFO("trying again: TRY " << trycounter);
              haserror=false;
           }

           // get metadata from s3
           S3FS_TRY

             // check if we have that path without first /
             HeadResponsePtr lRes;
             S3_LOG_DEBUG(" making head request to " << lpath.substr(1));
             lRes = lCon->head(theBucketname, lpath.substr(1));
             map_t lMap = lRes->getMetaData();
             if (theLogLevel <= S3_DEBUG) {
               S3_LOG_DEBUG("  requested metadata for " << lpath.substr(1));
               for (map_t::iterator lIter = lMap.begin(); lIter != lMap.end(); ++lIter) {
                 S3_LOG_DEBUG("    got " << (*lIter).first << " : " << (*lIter).second);
               }
               S3_LOG_DEBUG("    content-length: " << lRes->getContentLength());
             }

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

					 S3_LOG_ERROR("finally failed after " << trycounter << " tries");
           
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
    S3_LOG_ERROR("An Error occured while trying to get file attributes.");

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
  S3_LOG_DEBUG("path: " << path << " mode: " << mode);

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
    S3_LOG_DEBUG("path: " << path << " time:" << time_to_string(tv->tv_sec));
  }else{
    S3_LOG_DEBUG("path: " << path);
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
  S3_LOG_DEBUG("path: " << path << " uid:" << uid << " gid:" << gid);

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
  S3_LOG_DEBUG("path: " << path << " offset:" << offset);

// initialize result
  int result=0;
  std::string lpath(path);
#ifdef S3FS_USE_MEMCACHED
  std::string key;
#endif // S3FS_USE_MEMCACHED
  fuse_file_info fileinfo;
  memset(&fileinfo, 0, sizeof(struct fuse_file_info));

  try{
    if(offset==0){
      //get file stat
      struct stat stbuf;
      s3_getattr(path,&stbuf);

      std::auto_ptr<FileHandle> fileHandle(new FileHandle);

      // generate temp file and open it
      checkTempFolder();
      int ltempsize=theS3FSTempFilePattern.length();
      char ltempfile[ltempsize];
      strcpy(ltempfile,theS3FSTempFilePattern.c_str());
      fileHandle->id=mkstemp(ltempfile);
      fileHandle->filename = std::string(ltempfile);
      S3_LOG_DEBUG("File Descriptor # is: " << fileHandle->id << " file name = " << ltempfile);
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
      fileinfo.fh = (uint64_t)fileHandle->id;
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
      s3_release(path, &fileinfo);

    }else{
      S3_LOG_ERROR("Truncate only implemented for 0.");
    }

    return result;
  }catch(...){
    S3_LOG_ERROR("An Error occured while trying to open a file.");

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
  S3_LOG_DEBUG("path: " << path << " mode: " << mode);

  int result=0;

  std::string lpath(path);

  S3ConnectionPtr lCon = getConnection();
  bool haserror=false;
  unsigned int trycounter=0;

  try{
    do{
      trycounter++;
      haserror=false;
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
    S3_LOG_ERROR("An Error occured while trying make dir.");

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
  S3_LOG_DEBUG("path: " << path);

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
       S3_LOG_DEBUG("[Memcached] found entries for folder '" << lpath.substr(1) << "': " << value);
       return -ENOTEMPTY;
     }else if(rc==MEMCACHED_SUCCESS && value.length()==0){
       // folder empty -> can be removed
       S3_LOG_DEBUG("[Memcached] folder '" << lpath.substr(1) << "' is empty.");
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
         haserror=false;
         S3FS_TRY
           std::string lMarker;
           do {
             // get object without first /
             S3_LOG_DEBUG("list bucket: "<<theBucketname<<" prefix: "<<lpath.substr(1));
             lRes = lCon->listBucket(theBucketname, lpath.substr(1), lMarker, "/", -1);
             lRes->open();
             ListBucketResponse::Object o;
             while (lRes->next(o)) {
               struct stat lStat;
               memset(&lStat, 0, sizeof(struct stat));
               S3_LOG_DEBUG("result: " << o.KeyValue);
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
      haserror=false;
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
    S3_LOG_ERROR("An Error occured while trying to remove dir.");

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
  S3_LOG_DEBUG("readdir: " << path);

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
      S3_LOG_DEBUG("[Memcached] found entries for folder '" << lpath.substr(1) << "': " << value);
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
        haserror=false;
        ListBucketResponsePtr lRes;
        S3FS_TRY
          std::string lMarker;
          do {
            // get object without first /
            S3_LOG_DEBUG("list bucket: "<<theBucketname<<" prefix: "<<lpath.substr(1));
            lRes = lCon->listBucket(theBucketname, lpath.substr(1), lMarker, "/", -1);
            lRes->open();
            ListBucketResponse::Object o;
            while (lRes->next(o)) {
              struct stat lStat;
              memset(&lStat, 0, sizeof(struct stat));

              S3_LOG_DEBUG("  result: " << o.KeyValue);
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
    S3_LOG_ERROR("An Error occured while trying to read dir contents.");

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
  S3_LOG_DEBUG("path: " << path << " mode: " << mode);

  std::string lpath(path);
  std::auto_ptr<FileHandle> fileHandle(new FileHandle);

  // we ignore the passed mode
  mode_t lmode;
  if ( (mode & S_IFMT)==S_IFLNK){
    S3_LOG_DEBUG("creating symbolic link");
    lmode = S_IFLNK | 0777;
  }else{
    S3_LOG_DEBUG("creating standard file");
    lmode = S_IFREG | 0777;
  }

  // initialize result
  int result=0;
  memset(fileinfo, 0, sizeof(struct fuse_file_info));

  try{

    // generate temp file and open it
    checkTempFolder();
    int ltempsize=theS3FSTempFilePattern.length();
    char ltempfile[ltempsize];
    strcpy(ltempfile,theS3FSTempFilePattern.c_str());
    fileHandle->id=mkstemp(ltempfile);
    S3_LOG_DEBUG("File Descriptor # is: " << fileHandle->id << " file name = " << ltempfile);
    std::auto_ptr<std::fstream> tempfile(new std::fstream());
    tempfile->open(ltempfile);

    fileHandle->filename = std::string(ltempfile);
    fileHandle->size = 0;
    fileHandle->s3key = lpath.substr(1);// cut off the first slash
    //TODO this is a hack
    fileHandle->mode = lmode;
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
    stbuf.st_mode = lmode;
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
    S3_LOG_ERROR("An Error occured while trying to create a new file.");

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

  S3_LOG_DEBUG("path: " << path);

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
      haserror=false;
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

      // delete symbolic link target if it is existent
      key=theCache->getkey(AWSCache::PREFIX_SYMLINK,lpath.substr(1),"").c_str();
      theCache->delete_key(key);
    }
#endif // S3FS_USE_MEMCACHED

    S3FS_EXIT(result);
  }catch(...){
    S3_LOG_ERROR("An Error occured while trying to delete a file.");

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

  S3_LOG_DEBUG("path: " << path);

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
    checkTempFolder();
    int ltempsize=theS3FSTempFilePattern.length();
    char ltempfile[ltempsize];
    strcpy(ltempfile,theS3FSTempFilePattern.c_str());
    fileHandle->id=mkstemp(ltempfile);
    fileHandle->filename = std::string(ltempfile);
    S3_LOG_DEBUG("File Descriptor # is: " << fileHandle->id << " file name = " << ltempfile);
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
        haserror=false;
        S3_LOG_DEBUG("going to make get call to s3 for " << lpath.substr(1) << "; trycounter " << trycounter);
        S3FS_TRY
          GetResponsePtr lGet = lCon->get(theBucketname, lpath.substr(1));
          S3_LOG_DEBUG("successfully made get request");
          std::istream& lInStream = lGet->getInputStream();
          S3_LOG_DEBUG("received content with length: " << lGet->getContentLength());
          fileHandle->size=lGet->getContentLength();

          S3_LOG_DEBUG("going to write data to tempfile");
          // write data to temp file
          char data[1024];
          while (lInStream.good())     // loop while extraction from file is possible
          {
            lInStream.read(data, 1024);       // get character from file
            tempfile->write(data, lInStream.gcount());
            S3_LOG_DEBUG("wrote " << lInStream.gcount() << "bytes to tempfile");
          }
          tempfile->flush();
          S3_LOG_DEBUG("finished writing to tempfile");

          fileHandle->filestream = tempfile.release();
          fileHandle->is_write = false;
          fileHandle->mtime = getCurrentTime();
          fileHandle->mode = stbuf.st_mode;
          fileHandle->s3key = lpath.substr(1);

          //remember tempfile
          fileinfo->fh = (uint64_t)fileHandle->id;
          int lTmpPointer = fileHandle->id;
          tempfilemap.insert( std::pair<int,struct FileHandle*>(lTmpPointer,fileHandle.release()) );
          S3_LOG_DEBUG("put tempfile into map");

        S3FS_CATCH(Get)
      }while(haserror && trycounter<AWS_TRIES_ON_ERROR);

#ifdef S3FS_USE_MEMCACHED
    }
#endif // S3FS_USE_MEMCACHED
    if (result!=0){
      S3_LOG_DEBUG("setting the fileinfo filehandle to NULL");
      fileinfo->fh = NULL;
    }
    S3_LOG_DEBUG("returning with result " << result);
    return result;
  }catch(...){
    S3_LOG_ERROR("An Error occured while trying to open a file.");

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
  S3_LOG_DEBUG("path: " << path << " data: " << data << " size: " << size << " offset: " << offset);

  S3_LOG_DEBUG("data size: " << strlen(data));

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
      S3_LOG_ERROR("No temporary file handle exists.");
      return -EIO;
    }
    return result;
  }catch(...){
    S3_LOG_ERROR("An Error occured while trying write data to a file.");

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

  S3_LOG_DEBUG("path: " << path);

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
            haserror=false;
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
            S3_LOG_ERROR("saving file on s3 failed");
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
        S3_LOG_INFO("couldn't find filehandle.");
      }
    }else{
      S3_LOG_INFO("no fileinfo or filehandle-ID provided.");
    }
    return result;
  }catch(...){
    S3_LOG_ERROR("An Error occured while trying to release a file.");

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
  S3_LOG_DEBUG("path: " << path << " offset: " << offset << " size: " << size);
  S3_LOG_DEBUG("size of the tempfilemap " << tempfilemap.size());

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
    S3_LOG_DEBUG("readsize: " << readsize << " tempfile->gcount(): " << tempfile->gcount());
    assert(readsize == tempfile->gcount());
    return readsize;
  }catch(...){
    S3_LOG_ERROR("An Error occured while trying to read a file.");

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
  S3_LOG_DEBUG("path: " << path);

  // by default allow access
  return 0;
}

/*
 * Create a symbolic link
 *
 */
static int
s3_symlink(const char * oldpath, const char * newpath) 
{
  S3_LOG_DEBUG("oldpath: " << oldpath << " newpath: " << newpath);
  std::string lpath(newpath);
  int result=0;
  std::string key;
  
  try{

    // tell s3_create to create a symlink
    mode_t mode= S_IFLNK | 0777;
    fuse_file_info fileinfo;
    memset(&fileinfo, 0, sizeof(struct fuse_file_info));

    S3_LOG_DEBUG("create " << newpath);
    result=s3_create(newpath, mode, &fileinfo);

    // write the target into the created file
    if(result==0){
      S3_LOG_DEBUG("write");
      result=s3_write(newpath, oldpath, strlen(oldpath), 0, &fileinfo);

      // release it to s3
      S3_LOG_DEBUG("release " << newpath);
      result=s3_release(newpath, &fileinfo);
    }

#ifdef S3FS_USE_MEMCACHED
    if(result==0){ 

      // we only have to remember the link, anything else is managed by s3_create...
      key=theCache->getkey(AWSCache::PREFIX_SYMLINK,lpath.substr(1),"").c_str();
      theCache->save_key(key, oldpath);
    }
#endif //USE_MEMCACHED

  }catch(...){
    S3_LOG_ERROR("An Error occured while trying to create symlink " << newpath << " to file/folder " << oldpath );

#ifdef S3FS_USE_MEMCACHED

    // cleanup cache to prevent future errors
    key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);
    key=theCache->getkey(AWSCache::PREFIX_SYMLINK,lpath.substr(1),"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    return -EIO; // I/O Error
  }

  return result;
}


/*
 * Read the target of a symbolic link
 *
 * The buffer should be filled with a null terminated string. The buffer size 
 * argument includes the space for the terminating null character. If the linkname
 * is too long to fit in the buffer, it should be truncated. The return value should
 * be 0 for success.
 */ 
static int
s3_readlink(const char * path, char * link, size_t size)
{
  S3_LOG_DEBUG("path: " << path << " buffer size: " << sizeof(link));
  std::string lpath(path);
  int result=0;
  std::string key;
  int readsize=0;

  try{
#ifdef S3FS_USE_MEMCACHED
    std::string value;
    memcached_return rc;
    bool readlink=false;

    // check if the cache knows if the link exists
    key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    value=theCache->read_key(key, &rc);
    if (value.length() > 0 && value.compare("0")==0) // link does not exist
    {
      S3_LOG_DEBUG("[Memcached] link: " << lpath.substr(1) << " is marked as non existent in cache.");
      return -ENOENT;
    }else if(value.length() > 0 && value.compare("1")==0) // link does exist
    {
      S3_LOG_DEBUG("[Memcached] link: " << lpath.substr(1) << " is marked as existent in cache.");

      // get the target link
      key=theCache->getkey(AWSCache::PREFIX_SYMLINK,lpath.substr(1),"");
      value=theCache->read_key(key, &rc);
      if (value.compare("")==0){
      	
      	// although the link was marked as existent in cache the target value was not in the cache, so it needs to be read from s3
      	readlink=true;
      }else{      
      	
      	//found the target value in the cache
        strcpy(link,value.c_str());
      }
    }else{
    		
    		// existence of link is not marked in the cache
    		readlink=true;
    } 
    
    if(readlink){
#endif
      // open the file that contains the target path info
      fuse_file_info fileinfo;
      memset(&fileinfo, 0, sizeof(struct fuse_file_info));
      S3_LOG_DEBUG("open " << path);
      result=s3_open(path, &fileinfo);

      if(result==0){
        // read the target path
        S3_LOG_DEBUG("read " << path);
        readsize=s3_read(path,link,size-1,0,&fileinfo);
        
        // the fuse doc says: If the linkname is too long to fit in the buffer, it should be truncated. 
        // therefore we don't care for links that are longer than size
        // null termination
        link[readsize]='\0';
        S3_LOG_DEBUG("link " << link);

        // release the file
        S3_LOG_DEBUG("release " << path);
        result=s3_release(path, &fileinfo);
      }

#ifdef S3FS_USE_MEMCACHED
      if(result==0){ 

        // we only have to remember the link, anything else is managed by s3_create...
        key=theCache->getkey(AWSCache::PREFIX_SYMLINK,lpath.substr(1),"").c_str();
        theCache->save_key(key, link);
      }
    }
#endif //USE_MEMCACHED

  }catch(...){
    S3_LOG_ERROR("An Error occured while trying to read symlink " << path);

#ifdef S3FS_USE_MEMCACHED

    // cleanup cache to prevent future errors
    key=theCache->getkey(AWSCache::PREFIX_EXISTS,lpath.substr(1),"");
    theCache->delete_key(key);
    key=theCache->getkey(AWSCache::PREFIX_SYMLINK,lpath.substr(1),"");
    theCache->delete_key(key);
#endif // S3FS_USE_MEMCACHED

    return -EIO; // I/O Error
  }
  return result;
}




int
main(int argc, char **argv)
{
  // set callback functions
  s3_filesystem_operations.getattr    = s3_getattr;
  s3_filesystem_operations.utimens    = s3_utimens;
  s3_filesystem_operations.truncate   = s3_truncate;
  s3_filesystem_operations.chmod      = s3_chmod;
  s3_filesystem_operations.chown      = s3_chown;
  s3_filesystem_operations.mkdir      = s3_mkdir;
  s3_filesystem_operations.rmdir      = s3_rmdir;
  s3_filesystem_operations.readdir    = s3_readdir;
  s3_filesystem_operations.create     = s3_create;
  s3_filesystem_operations.unlink     = s3_unlink;
  s3_filesystem_operations.opendir    = s3_opendir;
  s3_filesystem_operations.read       = s3_read;
  s3_filesystem_operations.write      = s3_write;
  s3_filesystem_operations.open       = s3_open;
  s3_filesystem_operations.release    = s3_release;
  s3_filesystem_operations.symlink    = s3_symlink;
  s3_filesystem_operations.readlink   = s3_readlink;

  // handle s3fs and fuse args
  struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
  struct s3fs_config conf;
  memset(&conf, 0, sizeof(conf));
  fuse_opt_parse(&args, &conf, s3fs_opts, s3fs_opt_proc);

  // read from config file specified
  if (conf.property_file) {
    S3_LOG_INFO("reading config file " << conf.property_file);
    s3fs::utils::PropertyUtil::PropertyMapT lProperties;
    s3fs::utils::PropertyUtil::read(conf.property_file, lProperties);       
    thePropertyFile = conf.property_file;

    if (!conf.access_key)
      theAccessKeyId      = lProperties[s3fs::utils::Properties::AWS_ACCESS_KEY];
    if (!conf.secret_key)
      theSecretAccessKey  = lProperties[s3fs::utils::Properties::AWS_SECRET_ACCESS_KEY];
    if (!conf.temp_dir)
      theS3FSTempFolder   = lProperties[s3fs::utils::Properties::TEMP_DIR];
    if (!conf.bucket)
      theBucketname       = lProperties[s3fs::utils::Properties::BUCKET_NAME];
#ifdef S3FS_USE_MEMCACHED
    if (!conf.memcached_servers)
      theMemcachedServers = lProperties[s3fs::utils::Properties::MEMCACHED_SERVERS];
#endif
  } 

  // command line parameters override config file
  if (conf.access_key)
    theAccessKeyId = conf.access_key;
  if (conf.secret_key)
    theSecretAccessKey = conf.secret_key;
  if (conf.temp_dir)
    theS3FSTempFolder = conf.temp_dir;
  if (conf.bucket)
    theBucketname = conf.bucket;
#ifdef S3FS_USE_MEMCACHED
  if (conf.memcached_servers)
    theMemcachedServers = conf.memcached_servers;
#endif
  if (0 <= conf.log_level && conf.log_level <= 2)
    theLogLevel = (LogLevel) conf.log_level; 

#ifdef S3FS_LOG_SYSLOG
  openlog ("s3fs ", LOG_PID, LOG_DAEMON);
#endif
  if (theBucketname.length() == 0) {
    S3_LOG_ERROR("Please specify a S3 bucket (-o bucket=string).");
    std::cerr << "Please specify a S3 bucket (-o bucket=string)." << std::endl;
    return 4;
  }
  std::string lSyslogId = "s3fs " + theBucketname + " ";
#ifdef S3FS_LOG_SYSLOG
  openlog (lSyslogId.c_str(), LOG_PID, LOG_DAEMON);
#endif

  // error checking
  if (theAccessKeyId.length() == 0) {
    S3_LOG_ERROR("Please specify your aws access key (-o access-key=string).");
    std::cerr << "Please specify your aws access key (-o access-key=string)." << std::endl;
    return 1;
  }
  if (theSecretAccessKey.length() == 0) {
    S3_LOG_ERROR("Please specify your aws secret access key (-o secret-key=string).");
    std::cerr << "Please specify your aws secret access key (-o secret-key=string)." << std::endl;
    return 2;
  }
  if (theS3FSTempFolder.length() == 0) {
    S3_LOG_ERROR("Please specify a temporary directory (-o temp-dir=string).");
    std::cerr << "Please specify a temporary directory (-o temp-dir=string)." << std::endl;
    return 3;
  }
#ifdef S3FS_USE_MEMCACHED
  if (theMemcachedServers.length() == 0) {
    S3_LOG_ERROR("Please specify the memcached servers (-o memcached-servers=string).");
    std::cerr << "Please specify the memcached servers (-o memcached-servers=string)." << std::endl;
    return 5;
  }
  setenv("MEMCACHED_SERVERS", theMemcachedServers.c_str(), 1); // replace the MEMCACHED_SERVERS var in the environment
#endif

  theS3FSTempFilePattern = theS3FSTempFolder;
  if (theS3FSTempFolder.at(theS3FSTempFolder.length()-1) != '/')
    theS3FSTempFilePattern.append("/");
  theS3FSTempFilePattern.append("s3fs_file_XXXXXX");

#ifdef S3FS_USE_MEMCACHED
  theCache.reset(new AWSCache(theBucketname));
#endif //S3FS_USE_MEMCACHED

  // initialization
  theFactory = AWSConnectionFactory::getInstance();

  theS3ConnectionPool.reset(new ConnectionPool<S3ConnectionPtr>(CONNECTION_POOL_SIZE, theAccessKeyId, theSecretAccessKey));

  // test the credentials and the connection
  {
    try {
      S3ConnectionPtr lCon = getConnection();
      ListBucketResponsePtr lRes = lCon->listBucket(theBucketname, "", "", "/", -1);
      lRes->open();
      ListBucketResponse::Object o;
      while (lRes->next(o)) { }
      lRes->close();
     } catch (aws::AuthenticationException& auth_exception) {
       S3_LOG_ERROR("couldn't authenticate with s3 " << auth_exception.what());
       std::cerr << auth_exception.what() << std::endl;
       return 6;
     } catch (aws::AWSException& e) {
       S3_LOG_ERROR("error talking to s3 " << e.what());
       std::cerr << e.what() << std::endl;
       return 6;
     }
  }

  // check if mouting dir exists
  {
    std::string mount_dir=argv[1];
    struct stat st;
    if (stat(mount_dir.c_str(), &st) == -1) {
       if(conf.create_mount_dir){
          S3_LOG_INFO("mount dir " << mount_dir << "does not exist. Option create-mountdir is set. Trying to create folder.");
          if (::mkdir(mount_dir.c_str(),0777) == -1) {
             S3_LOG_ERROR("creating directory " << mount_dir << " failed");
             return 7;
          } else {
             S3_LOG_INFO("successfully created directory " << mount_dir);
          }
       }else{
          S3_LOG_ERROR("mount dir " << mount_dir << "does not exist. Option create-mountdir not set. mounting failed.");
          return 8;
       }
    } 
  }
  S3_LOG_INFO("mounting bucket " << theBucketname << " to " << argv[1]);

  return fuse_main(args.argc, args.argv, &s3_filesystem_operations, NULL);
}
