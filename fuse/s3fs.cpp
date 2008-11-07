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
#define USE_MEMCACHED 1


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
#include <cassert>
#include <stdlib.h>
#include <memory>
#include <cassert>

#include <libaws/aws.h>

#ifdef USE_MEMCACHED
#include <memcached.h>
#endif

using namespace aws;

struct FileHandle {
   FileHandle();
   ~FileHandle();
   int id;
   std::fstream* filestream;
   std::string filename;
   std::string s3key;
   int size;
   bool is_write; 
   mode_t mode;
   time_t mtime;
};

static AWSConnectionFactory* theFactory;
static ConnectionPool<S3ConnectionPtr>* theS3ConnectionPool;
static unsigned int CONNECTION_POOL_SIZE=5;

static std::string theAccessKeyId;
static std::string theSecretAccessKey;
static std::string theS3FSTempFolder;
static std::string BUCKETNAME("msb");

static std::string PREFIX_EXISTS("ex");
static std::string PREFIX_STAT_ATTR("attr");
static std::string PREFIX_DIR_LS("ls");
static std::string PREFIX_FILE("file");

static std::string DELIMITER_FOLDER_ENTRIES=",";
static unsigned int FILE_CACHING_UPPER_LIMIT=50000; // 1000 (means approx. 1kb)

static std::map<int,struct FileHandle*> tempfilemap;

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
      S3_LOG(S3_ERROR,location, s3Exception.what()); \
      result=-ENOENT;\
    } catch (AWSConnectionException & conException) { \
     S3_LOG(S3_ERROR,location,conException.what()); \
      result=-ECONNREFUSED; \
    }catch (AWSException & awsException) { \
      S3_LOG(S3_ERROR,location,"AWSException: " << awsException.what()); \
      result=-ENOENT;\
    }
#else
#  define S3FS_CATCH(kind) \
    } catch (kind ## Exception & s3Exception) { \
      result=-ENOENT;\
    } catch (AWSConnectionException & conException) { \
      result=-ECONNREFUSED; \
    }catch (AWSException & awsException) { \
      result=-ENOENT;\
    }
#endif


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

static long
to_long(const std::string s){
  return strtol(s.c_str(),NULL,10);
}

template <class T>
static std::string
to_string(T i)
{
  std::stringstream s;
  s << i;
  return s.str();
}

static void
to_vector(std::vector<std::string>& result, std::string to_split, const std::string& delimiter){
  size_t pos;
  while(to_split.length()>0){
     if(to_split.compare(delimiter)==0){
        break; //that's weird -> end here
     }
     pos=to_split.find(delimiter);
     if(pos==std::string::npos){
        result.push_back(to_split);
        to_split=""; // thats it
     }else{
        result.push_back(to_split.substr(0,pos));
        if(to_split.substr(pos).length()>1){
          to_split=to_split.substr(pos+1);
        }else{
          to_split=""; // that's it
        }
     }
  }
}

static std::string
getParentFolder(const std::string& path)
{
  size_t pos;
  pos=path.find_last_of("/");
  if(pos==std::string::npos){
    return "";
  }else{
    return path.substr(0,pos);
  }
}


#ifdef USE_MEMCACHED
/*******************
 * MEMCACHED HELPERS
 *******************
 */

static memcached_st *
get_Memcached_struct()
{
#ifndef NDEBUG
  std::string location="get_Memcached_struct(...)";
  S3_LOG(S3_DEBUG,location,"create struct");
#endif

    char *temp;
    memcached_server_st *servers;

    memcached_st * memc=memcached_create(NULL);

    if (!(temp= getenv("MEMCACHED_SERVERS")))
    {
      std::cerr << "Unabel to use memcached client functionality. Please specify the MEMCACHED_SERVERS environment variable" << std::endl;
      exit(4);
    }

    servers= memcached_servers_parse(strdup(temp));
    memcached_server_push(memc, servers);
    memcached_server_list_free(servers);

    return memc;
}

static void
free_Memcached_struct(memcached_st * memc)
{
#ifndef NDEBUG
  std::string location="free_Memcached_struct(...)";
  S3_LOG(S3_DEBUG,location,"destroy struct");
#endif

  memcached_free(memc);
}

static std::string 
getkey(std::string& prefix, std::string key, std::string attr)
{
   std::string result="";
   result.append(prefix);
   result.append(":");
   result.append(attr);
   result.append(":");

   // cut last slash
   if(key.length()>1 && key.at(key.length()-1)=='/'){
      key=key.substr(0,key.length()-1);
   }
   result.append(key);
   return result;
}

static void
delete_cache_key(memcached_st* memc, std::string& key)
{
#ifndef NDEBUG
  std::string location="delete_cache_key(...) [Memcached]";
#endif
  memcached_return rc;

  rc=memcached_delete (memc, key.c_str(), strlen(key.c_str()),(time_t)0);

#ifndef NDEBUG
  if (rc == MEMCACHED_SUCCESS){
     S3_LOG(S3_DEBUG,location,"   successfully invalidated key: '" << key << "'");
  }else{
     S3_LOG(S3_INFO,location,"    [ERROR] could not delete key: '" << key << "' from cache (rc=" << (int) rc << ": "<< memcached_strerror(memc,rc) <<")");
  }
#endif
}


static void
save_cache_key(memcached_st* memc, std::string& key, const std::string& value)
{
#ifndef NDEBUG
  std::string location="save_cache_key(...) [Memcached]";
#endif
  memcached_return rc;

  rc=memcached_set(memc, key.c_str(), strlen(key.c_str()),value.c_str(), strlen(value.c_str()),(time_t)0, (uint32_t)0);
#ifndef NDEBUG
  if (rc == MEMCACHED_SUCCESS){
     S3_LOG(S3_DEBUG,location,"   successfully stored key: '" << key << "' value: '" << value << "'");
  }else{
     S3_LOG(S3_INFO,location,"    [ERROR] could not store key: '" << key << "' in cache (rc=" << (int) rc << ": "<< memcached_strerror(memc,rc) <<")");
  }
#endif
}

static void
save_cache_key_file(memcached_st* memc, std::string& key, std::fstream* fstream, size_t size)
{
#ifndef NDEBUG
  std::string location="save_cache_key_file(...) [Memcached]";
#endif
  memcached_return rc;
  std::auto_ptr<char> memblock(new char [size]);

  assert(fstream);
  fstream->seekg(0);

  fstream->read(memblock.get(),size);

  if(size==0){
    rc=memcached_set(memc, key.c_str(), strlen(key.c_str()), "", 0,(time_t)0, (uint32_t)0);
  }else{
    rc=memcached_set(memc, key.c_str(), strlen(key.c_str()), memblock.get(), size,(time_t)0, (uint32_t)0);
  }

#ifndef NDEBUG
  if (rc == MEMCACHED_SUCCESS){
     std::string lvalue(memblock.get());
     S3_LOG(S3_DEBUG,location,"   successfully stored file: '" << key << "' value: '"<<lvalue<<"'");
  }else{
     S3_LOG(S3_INFO,location,"    [ERROR] could not store file: '" << key << "' in cache (rc=" << (int) rc << ": "<< memcached_strerror(memc,rc) <<")");
  }
#endif
}

static void
store_cache_stat(struct stat* stbuf,std::string path)
{
  // get memc
  memcached_st* memc=get_Memcached_struct();

  std::string key=getkey(PREFIX_STAT_ATTR,path,"mode");
  save_cache_key(memc, key, to_string(stbuf->st_mode));

  key=getkey(PREFIX_STAT_ATTR,path,"gid").c_str();
  save_cache_key(memc, key,to_string(stbuf->st_gid));

  key=getkey(PREFIX_STAT_ATTR,path,"oid").c_str();
  save_cache_key(memc, key,to_string(stbuf->st_uid));

  key=getkey(PREFIX_STAT_ATTR,path,"mtime").c_str();
  save_cache_key(memc, key,to_string(stbuf->st_mtime));

  key=getkey(PREFIX_STAT_ATTR,path,"size").c_str();
  save_cache_key(memc, key,to_string(stbuf->st_size));

  key=getkey(PREFIX_STAT_ATTR,path,"nlink").c_str();
  save_cache_key(memc, key,to_string(stbuf->st_nlink));

  free_Memcached_struct(memc);

}

static std::string
read_cached_key(memcached_st* memc, std::string& key, memcached_return* rc)
{
#ifndef NDEBUG
  std::string location="read_cached_key(...) [Memcached]";
#endif
  uint32_t flags;
  size_t value_length;
  char* value;
  std::string lvalue="";

  value=memcached_get(memc, key.c_str(), strlen(key.c_str()),&value_length, &flags, rc);
  if (value!=NULL){
       lvalue=std::string(value);
  }

#ifndef NDEBUG
  std::string lkey(key);
  if (*rc == MEMCACHED_SUCCESS){
    S3_LOG(S3_DEBUG,location,"   successfully read cached key: '" << lkey << "' value: '" << lvalue << "'");
  }else{
    S3_LOG(S3_INFO,location,"    [ERROR] could not read key: '" << lkey << "' from cache (rc=" << (int) *rc << ": "<< memcached_strerror(memc,*rc) <<")");
  }
#endif

 return lvalue;
}

static void
read_cache_key_file(memcached_st* memc, std::string& key, std::fstream* fstream, memcached_return* rc)
{
#ifndef NDEBUG
  std::string location="read_cache_key_file(...) [Memcached]";
#endif
  uint32_t flags;
  size_t value_length;
  char* value;

  assert(fstream);

  value=memcached_get(memc, key.c_str(), strlen(key.c_str()),&value_length, &flags, rc);

#ifndef NDEBUG
  std::string lkey(key);
  if (*rc == MEMCACHED_SUCCESS){
    if(value!=NULL){
      (*fstream) << value;
      fstream->flush();
    }
    S3_LOG(S3_DEBUG,location,"   successfully read cached file: '" << lkey << "'");
  }else{
    S3_LOG(S3_INFO,location,"    [ERROR] could not read file: '" << lkey << "' from cache (rc=" << (int) *rc << ": "<< memcached_strerror(memc,*rc) <<")");
  }
#endif

}


static void
read_cached_stat(struct stat* stbuf,std::string path)
{
  // get memc
  memcached_st* memc=get_Memcached_struct();
  memcached_return rc;
  
  std::string key=getkey(PREFIX_STAT_ATTR,path,"mode");
  stbuf->st_mode=atoi(read_cached_key(memc, key, &rc).c_str());

  key=getkey(PREFIX_STAT_ATTR,path,"gid");
  stbuf->st_gid=atoi(read_cached_key(memc, key, &rc).c_str());

  key=getkey(PREFIX_STAT_ATTR,path,"oid");
  stbuf->st_uid=atoi(read_cached_key(memc, key, &rc).c_str());

  key=getkey(PREFIX_STAT_ATTR,path,"mtime");
  stbuf->st_mtime=atol(read_cached_key(memc, key, &rc).c_str());

  key=getkey(PREFIX_STAT_ATTR,path,"size");
  stbuf->st_size=atol(read_cached_key(memc, key, &rc).c_str());

  key=getkey(PREFIX_STAT_ATTR,path,"nlink");
  stbuf->st_nlink=atol(read_cached_key(memc, key, &rc).c_str());

  free_Memcached_struct(memc);

}
#endif //#ifdef USE_MEMCACHED

/* END: Memcached helpers
 *************************/




static void
fill_stat(map_t& aMap, struct stat* stbuf, long long aContentLength)
{
  stbuf->st_mode |= to_int(aMap["mode"]);
  stbuf->st_gid  |= to_int(aMap["gid"]);
  stbuf->st_uid  |= to_int(aMap["oid"]);
  stbuf->st_mtime  |= to_long(aMap["mtime"]);
  
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
 * FileHandle struct constructor and destructor
 */
FileHandle::FileHandle()
{
  id=-1;
  filename="";
}

FileHandle::~FileHandle()
{
#ifndef NDEBUG
  std::string location="FileHandle::~FileHandle()";
  S3_LOG(S3_DEBUG,location,"destroying filehandle: " << to_string(id));
#endif

  if(id!=-1)
     tempfilemap.erase(id);
  if(filestream)
     delete filestream;filestream=0;

  // delete tempfilename if existent
  if(!filename.empty()){
    remove(filename.c_str());
  }
}

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
//  S3_LOG(S3_DEBUG,location,"getattr path: " << path);
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
    lMap["gid"]=to_string(getgid());
    lMap["uid"]=to_string(getuid());
    lMap["dir"]="1";
    fill_stat(lMap, stbuf, 4096);

#ifndef NDEBUG
    S3_LOG(S3_DEBUG,location,"  requested getattr for root / => exit");
#endif
    result=0;
  }else{

       std::string lpath(path);

#ifdef USE_MEMCACHED
   std::string value;

   // get memc
   memcached_st* memc=get_Memcached_struct();
   memcached_return rc;

   std::string key=getkey(PREFIX_EXISTS,lpath.substr(1),"");

    value=read_cached_key(memc, key, &rc);
    if (value.length() > 0 && value.compare("0")==0) // file does not exist
    {
      S3_LOG(S3_DEBUG,location,"[Memcached] file or folder: " << lpath.substr(1) << " is marked as non exitent in cache.");
      return -ENOENT;
    }else if(value.length() > 0 && value.compare("1")==0) // file does exist
    {
      S3_LOG(S3_DEBUG,location,"[Memcached] file or folder: " << lpath.substr(1) << " is marked as exitent in cache.");

      // get attributes from cache
      read_cached_stat(stbuf,lpath.substr(1));
    }
    else 
    {
#endif

       // file or folder not known in cache, so check with s3
       S3ConnectionPtr lCon = getConnection();
       S3FS_TRY

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

#ifdef USE_MEMCACHED
       if(result==-ENOENT){ 

         // remember in cache that file does not exist
         key=getkey(PREFIX_EXISTS,lpath.substr(1),"").c_str();
         save_cache_key(memc, key, "0");
      }else{

         //remember successfully retrieved data in cache
         save_cache_key(memc, key, "1");
         store_cache_stat(stbuf, lpath.substr(1));
      }
#endif

       S3FS_EXIT(result);

#ifdef USE_MEMCACHED
       }
       free_Memcached_struct(memc);
#endif

    }

  return result;
}


/*
 * Change the permission bits of a file
 */
static int
s3_chmod(const char * path, mode_t mode)
{
#ifndef NDEBUG
  std::string location="s3_chmod(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " mode: " << mode);
#endif

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
#ifndef NDEBUG
  std::string location="s3_utimens(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " time:" << tv);
#endif

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
#ifndef NDEBUG
  std::string location="s3_chown(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " uid:" << uid << " gid:" << gid);
#endif

  //init result
  int result=0;

  // TODO set new owner

  return result;

}


/*
 * Change the size of a file
 *
 * This function is not needed as the size is changed anyway when
 * putting an object on s3
 */
static int 
s3_truncate(const char * path, off_t offset)
{
#ifndef NDEBUG
  std::string location="s3_truncate(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " offset:" << offset);
#endif

  //init result
  int result=0;

  return result;

}

/*
 * Set extended attributes
 *
 */
/*static int
s3_setxattr(const char * path, const char * char2, const char * char3, size_t size, int int1)
{
#ifndef NDEBUG
  std::string location="s3_setxattr(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " char2: " << char2 << " char3: " << char3 << " size: " << size << " int1: " << int1);
#endif

   // init result
   int result=0;

   //TODO 

   return result;
} */


static int
s3_mkdir(const char *path, mode_t mode)
{
#ifndef NDEBUG
  std::string location="s3_mkdir(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " mode: " << mode);
#endif
  int result=0;

  std::string lpath(path);

  S3ConnectionPtr lCon = getConnection();
  S3FS_TRY
    map_t lDirMap;
    lDirMap.insert(pair_t("dir", "1"));
    lDirMap.insert(pair_t("gid", to_string(getgid())));
    lDirMap.insert(pair_t("uid", to_string(getuid())));
    lDirMap.insert(pair_t("mode", to_string(mode)));
    lDirMap.insert(pair_t("mtime", to_string(time(NULL))));
    PutResponsePtr lRes = lCon->put(BUCKETNAME, lpath.substr(1), 0, "text/plain", 0, &lDirMap);

    // success

#ifdef USE_MEMCACHED
  // get memc
  memcached_st* memc=get_Memcached_struct();

  // delete data from cache
  std::string key=getkey(PREFIX_EXISTS,lpath.substr(1),"").c_str();
  delete_cache_key(memc, key);

  // delete cached dir entries of parent folder
  std::string parentfolder=getParentFolder(lpath.substr(1));
  key=getkey(PREFIX_DIR_LS,parentfolder,"").c_str();
  delete_cache_key(memc, key);

  //cleanup
  free_Memcached_struct(memc);
#endif // USE_MEMCACHED

    S3FS_EXIT(result);
  S3FS_CATCH(Put)

  S3FS_EXIT(result);
}



static int
s3_rmdir(const char *path)
{
#ifndef NDEBUG
  std::string location="s3_rmdir(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path);
#endif

  int result=0;
  std::string lpath(path);

  S3ConnectionPtr lCon = getConnection();

  try {
    HeadResponsePtr lRes;
    lRes = lCon->head(BUCKETNAME, lpath.substr(1));
    map_t lMap = lRes->getMetaData();
    if (lMap.count("dir") == 0) {
#ifndef NDEBUG
      std::cerr << " not a directory" << std::endl;
#endif
      S3FS_EXIT(-ENOTDIR);  
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

  // now we have to check if the folder is empty
#ifdef USE_MEMCACHED
   std::string value;

   // get memc
   memcached_st* memc=get_Memcached_struct();
   memcached_return rc;

   std::string key=getkey(PREFIX_DIR_LS,lpath.substr(1),"");

    value=read_cached_key(memc, key, &rc);
    if (rc==MEMCACHED_SUCCESS && value.length()>0) // there are entries in the folder
    {
#ifndef NDEBU
      S3_LOG(S3_DEBUG,location,"[Memcached] found entries for folder '" << lpath.substr(1) << "': " << value);
#endif
      result=-ENOTEMPTY;
      S3FS_EXIT(result);
    }else if(rc==MEMCACHED_SUCCESS && value.length()==0){
       // folder empty
#ifndef NDEBU
      S3_LOG(S3_DEBUG,location,"[Memcached] folder '" << lpath.substr(1) << "' is empty.");
#endif
    }
    else 
    {
#endif
  std::string lentries="";
  S3ConnectionPtr lCon = getConnection();
  ListBucketResponsePtr lRes;
  if(lpath.length()>0 && lpath.at(lpath.length()-1)!='/') lpath.append("/");
  S3FS_TRY
    std::string lMarker;
    do {
      // get object without first /
#ifndef NDEBUG
      S3_LOG(S3_DEBUG,location,"  list bucket: "<<BUCKETNAME<<" prefix: "<<lpath.substr(1));
#endif
      lRes = lCon->listBucket(BUCKETNAME, lpath.substr(1), lMarker, "/", -1);
      lRes->open();
      ListBucketResponse::Object o;
      while (lRes->next(o)) {
        struct stat lStat;
        memset(&lStat, 0, sizeof(struct stat));

#ifndef NDEBUG
        S3_LOG(S3_DEBUG,location,"  result: " << o.KeyValue);
#endif
        std::string lTmp = o.KeyValue.replace(0, lpath.length()-1, "");

        // remember entries
        if(lentries.length()>0) lentries.append(DELIMITER_FOLDER_ENTRIES);
        lentries.append(lTmp);

        lMarker = o.KeyValue;
      }
      lRes->close();
    } while (lRes->isTruncated());

  S3FS_CATCH(ListBucket);

   if(result!=-ENOENT && lentries.length()>0){ 

     // folder not empty
     result=-ENOTEMPTY;

#ifdef USE_MEMCACHED
     //remember successfully retrieved entries in cache
     key=getkey(PREFIX_DIR_LS,lpath.substr(1),"").c_str();
     save_cache_key(memc, key, lentries);

     //cleanup
     free_Memcached_struct(memc);
#endif // USE_MEMCACHED

     S3FS_EXIT(result);
   }
 }

  // folder is empty -> can be deleted
  if(lpath.length()>0 && lpath.at(lpath.length()-1)=='/') lpath=lpath.substr(0,lpath.length()-1);
  S3FS_TRY
    DeleteResponsePtr lRes = lCon->del(BUCKETNAME, lpath.substr(1));
  S3FS_CATCH(Put)

#ifdef USE_MEMCACHED

  // delete data from cache
  key=getkey(PREFIX_EXISTS,lpath.substr(1),"").c_str();
  delete_cache_key(memc, key);

  key=getkey(PREFIX_DIR_LS,lpath.substr(1),"").c_str();
  delete_cache_key(memc, key);

  // delete cached dir entries of parent folder
  std::string parentfolder=getParentFolder(lpath.substr(1));
  key=getkey(PREFIX_DIR_LS,parentfolder,"").c_str();
  delete_cache_key(memc, key);

  //cleanup
  free_Memcached_struct(memc);
#endif // USE_MEMCACHED

  S3FS_EXIT(result);
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

  int result=0;

  filler(buf, ".", NULL, 0);
  filler(buf, "..", NULL, 0);

  // TODO handle full buffer by remembering the marker

  std::string lPath(path);
  if (lPath.at(lPath.length()-1) != '/')
    lPath += "/";

#ifdef USE_MEMCACHED
   std::string value;

   // get memc
   memcached_st* memc=get_Memcached_struct();
   memcached_return rc;

   std::string key=getkey(PREFIX_DIR_LS,lPath.substr(1),"");

    value=read_cached_key(memc, key, &rc);
    if (rc==MEMCACHED_SUCCESS) // there are entries in the cache for this folder
    {
      S3_LOG(S3_DEBUG,location,"[Memcached] found entries for folder '" << lPath.substr(1) << "': " << value);
      std::vector<std::string> items;
      std::vector<std::string>::iterator iter;

      to_vector(items,value,DELIMITER_FOLDER_ENTRIES);
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
  S3ConnectionPtr lCon = getConnection();
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

#ifdef USE_MEMCACHED
        // remember entries to store in cache
        if(lentries.length()>0) lentries.append(DELIMITER_FOLDER_ENTRIES);
        lentries.append(lTmp);
#endif //USE_MEMCACHED

        filler(buf, lTmp.c_str(), &lStat, 0);
        lMarker = o.KeyValue;
      }
      lRes->close();
    } while (lRes->isTruncated());

  S3FS_CATCH(ListBucket);

#ifdef USE_MEMCACHED
     if(result==-ENOENT){ 

         // remember in cache that no entries exist in folder
         save_cache_key(memc, key, "");
      }else{

         //remember successfully retrieved entries in cache
         save_cache_key(memc, key, lentries);
      }
#endif

       S3FS_EXIT(result);

#ifdef USE_MEMCACHED
  }
  free_Memcached_struct(memc);
#endif

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
#ifndef NDEBUG
  std::string location="s3_create(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " mode: " << mode);
#endif

  std::string lpath(path);

  std::auto_ptr<FileHandle> fileHandle(new FileHandle);

  // initialize result
  int result=0;
  memset(fileinfo, 0, sizeof(struct fuse_file_info));

  // generate temp file and open it
  int ltempsize=theS3FSTempFolder.length();
  char ltempfile[ltempsize];
  strcpy(ltempfile,theS3FSTempFolder.c_str());
  fileHandle->id=mkstemp(ltempfile);
#ifndef NDEBUG
  S3_LOG(S3_DEBUG,location,"File Descriptor # is: " << fileHandle->id << " file name = " << ltempfile);
#endif
  std::auto_ptr<std::fstream> tempfile(new std::fstream());
  tempfile->open(ltempfile);

  fileHandle->filename = std::string(ltempfile);
  fileHandle->size = 0;
  fileHandle->s3key = lpath.substr(1);// cut off the first slash
  fileHandle->mode = mode;
  fileHandle->filestream = tempfile.release();
  fileHandle->is_write = true;
  fileHandle->mtime = time(NULL);

  //remember filehandle
  fileinfo->fh = (uint64_t)fileHandle->id;
  int lTmpPointer = fileHandle->id;
  tempfilemap.insert( std::pair<int,struct FileHandle*>(lTmpPointer,fileHandle.release()) );

#ifdef USE_MEMCACHED
  // get memc
  memcached_st* memc=get_Memcached_struct();

  // init stat
  struct stat stbuf;
  //memset(stbuf, 0, sizeof(struct stat));
  stbuf.st_mode = S_IFREG | 0644;
  stbuf.st_gid = getgid();
  stbuf.st_uid = getuid();
  stbuf.st_mtime = time(NULL);
  stbuf.st_size = 0;
  stbuf.st_nlink = 1;

  // store data for newly created file to cache
  std::string key=getkey(PREFIX_EXISTS,lpath.substr(1),"").c_str();
  save_cache_key(memc, key, "1");
  store_cache_stat(&stbuf, lpath.substr(1));

  // delete cached dir entries of parent folder
  std::string parentfolder=getParentFolder(lpath.substr(1));
  key=getkey(PREFIX_DIR_LS,parentfolder,"").c_str();
  delete_cache_key(memc, key);

  free_Memcached_struct(memc);
#endif // USE_MEMCACHED

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
  std::string location="s3_unlink(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path);
#endif

  int result=0;
  std::string lpath(path);

  S3ConnectionPtr lCon = getConnection();

  try {
    HeadResponsePtr lRes;
    lRes = lCon->head(BUCKETNAME, lpath.substr(1));
    map_t lMap = lRes->getMetaData();
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
    DeleteResponsePtr lRes = lCon->del(BUCKETNAME, lpath.substr(1));
  S3FS_CATCH(Put)

#ifdef USE_MEMCACHED
  // get memc
  memcached_st* memc=get_Memcached_struct();

  // delete data from cache
  std::string key=getkey(PREFIX_EXISTS,lpath.substr(1),"").c_str();
  delete_cache_key(memc, key);

  // delete cached dir entries of parent folder
  std::string parentfolder=getParentFolder(lpath.substr(1));
  key=getkey(PREFIX_DIR_LS,parentfolder,"").c_str();
  delete_cache_key(memc, key);

  //cleanup
  free_Memcached_struct(memc);
#endif // USE_MEMCACHED

  S3FS_EXIT(result);
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

  //get file stat
  struct stat stbuf;
  s3_getattr(path,&stbuf);

  std::auto_ptr<FileHandle> fileHandle(new FileHandle);

  // initialize result
  int result=0;
  memset(fileinfo, 0, sizeof(struct fuse_file_info));

#ifdef USE_MEMCACHED
  //init
  std::string lpath(path);
  std::string key;
  bool got_file_cont_from_cache=false;

  // get memc
  memcached_st* memc=get_Memcached_struct();
  memcached_return rc;
#endif // USE_MEMCACHED

  // generate temp file and open it
  int ltempsize=theS3FSTempFolder.length();
  char ltempfile[ltempsize];
  strcpy(ltempfile,theS3FSTempFolder.c_str());
  fileHandle->id=mkstemp(ltempfile);
  fileHandle->filename = std::string(ltempfile);
#ifndef NDEBUG
  S3_LOG(S3_DEBUG,location,"File Descriptor # is: " << fileHandle->id << " file name = " << ltempfile);
#endif
  std::auto_ptr<std::fstream> tempfile(new std::fstream());
  tempfile->open(ltempfile);

#ifdef USE_MEMCACHED
   unsigned int filesize=(unsigned int)stbuf.st_size;

   // file can only be in cach if content is not too big
   if(filesize<=FILE_CACHING_UPPER_LIMIT){
     key=getkey(PREFIX_FILE,lpath.substr(1),"").c_str();
     read_cache_key_file(memc,key,dynamic_cast<std::fstream*>(tempfile.get()),&rc);
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
#endif // USE_MEMCACHED

  // now lets get the data and save it into the temp file
  S3ConnectionPtr lCon = getConnection();

  S3FS_TRY

    std::string lpath(path);
    GetResponsePtr lGet = lCon->get(BUCKETNAME, lpath.substr(1));
    std::istream& lInStream = lGet->getInputStream();

#ifndef NDEBUG
    S3_LOG(S3_DEBUG,location,"content length: " << lGet->getContentLength());
#endif
    fileHandle->size=lGet->getContentLength();

    // write data to temp file
    while (lInStream.good())     // loop while extraction from file is possible
    {
      (*tempfile) << (char) lInStream.get();       // get character from file
    }
    tempfile->flush();

    fileHandle->filestream = tempfile.release();
    fileHandle->is_write = false;
    fileHandle->mode = stbuf.st_mode;
    fileHandle->s3key = lpath.substr(1);

    //remember tempfile
    fileinfo->fh = (uint64_t)fileHandle->id;
    int lTmpPointer = fileHandle->id;
    tempfilemap.insert( std::pair<int,struct FileHandle*>(lTmpPointer,fileHandle.release()) );

  S3FS_CATCH(Get)

#ifdef USE_MEMCACHED
  }
  //cleanup
  free_Memcached_struct(memc);
#endif // USE_MEMCACHED

  return result;
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
#ifndef NDEBUG
  std::string location="s3_write(...)";
  S3_LOG(S3_DEBUG,location,"path: " << path << " data: " << data << " size: " << size << " offset: " << offset);
#endif

  // init result
  int result=0;

  FileHandle* fileHandle=tempfilemap.find((int)fileinfo->fh)->second;
  std::string tempfilename=fileHandle->filename;
  std::fstream* tempfile=fileHandle->filestream;

  // write data to temp file
  tempfile->seekp(offset);
  tempfile->write(data,size);

  // flag to update file on s3
  fileHandle->is_write = true;

  result=size;

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
  if(fileinfo!=NULL
        && (int)fileinfo->fh){

      // init
      std::string tempfilename="";

      // get filehandle struct
      std::map<int,struct FileHandle*>::iterator foundtempfile=tempfilemap.find((int)fileinfo->fh);
      if(foundtempfile!=tempfilemap.end()){
         std::auto_ptr<FileHandle> fileHandle(foundtempfile->second);

#ifdef USE_MEMCACHED
      //init
      std::string lpath(path);
      std::string key;

      // get memc
      memcached_st* memc=get_Memcached_struct();

#endif // USE_MEMCACHED

         // check if we have to send changes to s3
         if(fileHandle->is_write){

              // reset filestream
              fileHandle->filestream->seekg(0);

              // transfer temp file to s3
              S3ConnectionPtr lCon = getConnection();
              S3FS_TRY
                 map_t lDirMap;
                 lDirMap.insert(pair_t("file", "1"));
                 lDirMap.insert(pair_t("gid", to_string(getgid())));
                 lDirMap.insert(pair_t("uid", to_string(getuid())));
                 lDirMap.insert(pair_t("mode", to_string(fileHandle->mode)));
#ifndef NDEBUG
        S3_LOG(S3_DEBUG,location,"mode is "<<to_string(fileHandle->mode));
#endif
                 lDirMap.insert(pair_t("mtime", to_string(fileHandle->mtime)));
                 PutResponsePtr lRes = lCon->put(BUCKETNAME, fileHandle->s3key, *(fileHandle->filestream), "text/plain", &lDirMap);

#ifdef USE_MEMCACHED
                 // determine file size
                 /*unsigned int filesize;
                 fileHandle->filestream->seekg (0, std::ios::end);
                 filesize=(unsigned int) fileHandle->filestream->tellg();

                 // only cache file content if not too big
                 if(filesize<=FILE_CACHING_UPPER_LIMIT){
                   key=getkey(PREFIX_FILE,lpath.substr(1),"").c_str();
                   save_cache_key_file(memc,key,dynamic_cast<std::fstream*>(fileHandle->filestream),filesize); 
                 }*/
                 key=getkey(PREFIX_FILE,lpath.substr(1),"").c_str();
                 delete_cache_key(memc,key);

#endif // USE_MEMCACHED

               S3FS_CATCH(Put)

#ifdef USE_MEMCACHED
               if(result==-ENOENT){ 
#ifndef NDEBUG
                  S3_LOG(S3_ERROR,location,"saving file on s3 failed");
#endif
               }else{

                  // invalidate cached data for file because it changed
                  key=getkey(PREFIX_EXISTS,lpath.substr(1),"").c_str();
                  delete_cache_key(memc, key);
               }
#endif

         }else{ // we have to send no changes to s3 -> readonly

#ifdef USE_MEMCACHED
           // determine file size
           unsigned int filesize;
           fileHandle->filestream->seekg (0, std::ios::end);
           filesize=(unsigned int) fileHandle->filestream->tellg();

           // only cache file content if not too big
           if(filesize<=FILE_CACHING_UPPER_LIMIT){
              key=getkey(PREFIX_FILE,lpath.substr(1),"").c_str();
              save_cache_key_file(memc,key,dynamic_cast<std::fstream*>(fileHandle->filestream),filesize); 
           }

#endif // USE_MEMCACHED

            //close file stream if still open
            if(fileHandle->filestream && fileHandle->filestream->is_open()){
               fileHandle->filestream->close();
            }

         }

#ifdef USE_MEMCACHED
         //cleanup
         free_Memcached_struct(memc);
#endif // USE_MEMCACHED

      }else{
#ifndef NDEBUG
        S3_LOG(S3_INFO,location,"couldn't find filehandle.");
#endif
      }

  }else{
#ifndef NDEBUG
      S3_LOG(S3_INFO,location,"no fileinfo or filehandle-ID provided.");
#endif
  }
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

  FileHandle* fileHandle=tempfilemap.find((int)fileinfo->fh)->second;
  std::string tempfilename=fileHandle->filename;
  std::fstream* tempfile=fileHandle->filestream;
  
  // get length of file:
  unsigned int filelength = (unsigned int) fileHandle->size;
  
  int readsize = 0;
  if(size>filelength || (size-offset)>filelength){
    readsize=filelength-offset;
  }else{
    readsize=size;
  }
  
  tempfile->seekg(offset);
  memset(buf, 0, readsize); 
  tempfile->read(buf,readsize);
#ifndef NDEBUG
  S3_LOG(S3_DEBUG,location,"readsize: " << readsize << " tempfile->gcount(): " << tempfile->gcount());
#endif
  assert(readsize == tempfile->gcount());
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
  
  if(getenv("S3FS_TEMP")!=NULL){
    theS3FSTempFolder = getenv("S3FS_TEMP");
    theS3FSTempFolder.append("/s3fs_file_XXXXXX");
  }else{
    std::cerr << "Please specify the S3FS_TEMP environment variable. It should point to a folder where you have write access." << std::endl;
    return 3;
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


  theS3ConnectionPool=new ConnectionPool<S3ConnectionPtr>(CONNECTION_POOL_SIZE,theAccessKeyId,theSecretAccessKey);

  std::cerr << "####################" << std::endl;
  std::cerr << "####################" << std::endl;
  std::cerr << "# S3FS starting up #" << std::endl;
  std::cerr << "####################" << std::endl;
  std::cerr << "####################" << std::endl;


  // let's get started
  return fuse_main(argc, argv, &s3_filesystem_operations, NULL);
}
