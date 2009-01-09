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
#include "awscache.h"
#include <cassert>
#include <memory>
#include <syslog.h>

#define S3FS_LOG_SYSLOG 1

#ifndef NDEBUG
static int S3CACHE_DEBUG=0;
static int S3CACHE_INFO=1;
static int S3CACHE_ERROR=2;
static int S3CACHE_LOGGING_LEVEL=S3CACHE_INFO;

#  ifdef S3FS_LOG_SYSLOG
#    define S3CACHE_LOG(level,location,message) \
      if(S3CACHE_LOGGING_LEVEL <= level) \
      { \
	    std::ostringstream logMessage; \
            if (level==S3CACHE_DEBUG){ \
                    logMessage << "AWSCache(bucket:" << theBucketname << ") " << location << " [DEBUG] ## " << message << " ## "; \
		    syslog( LOG_DEBUG, logMessage.str().c_str() ); \
	    }else if (level==S3CACHE_INFO){ \
                    logMessage << "AWSCache(bucket:" << theBucketname << ") " << location << " [INFO] ## " << message << " ## "; \
		    syslog( LOG_NOTICE, logMessage.str().c_str() ); \
	    }else if (level==S3CACHE_ERROR){ \
                    logMessage << "AWSCache(bucket:" << theBucketname << ") " << location << " [ERROR] ## " << message << " ## "; \
		    syslog( LOG_ERR, logMessage.str().c_str() ); \
	    } \
      }
#  else
#    define S3CACHE_LOG(level,location,message) \
      if(S3CACHE_LOGGING_LEVEL <= level) \
      { \
	std::stringstream logMessage; \
	std::string levelstr=""; \
	if (level==S3CACHE_DEBUG){ \
		levelstr="DEBUG"; \
	}else if (level==S3CACHE_INFO){ \
		levelstr="INFO"; \
	}else if (level==S3CACHE_ERROR){ \
		levelstr="ERROR"; \
	} \
	logMessage << "[" << levelstr << "] " << location << " ## " << message << " ##";  \
	std::cerr << logMessage.str() << std::endl; \
      }
#  endif 
#else
#define S3CACHE_LOG(level,location,message)
#endif

#ifndef NDEBUG
#define ASSERT(assertion) \
   assert(assertion); 
#else
#define ASSERT(assertion)
#endif

namespace aws { 

  std::string AWSCache::PREFIX_EXISTS("ex");
  std::string AWSCache::PREFIX_STAT_ATTR("attr");
  std::string AWSCache::PREFIX_DIR_LS("ls");
  std::string AWSCache::PREFIX_FILE("file");

  unsigned int AWSCache::FILE_CACHING_UPPER_LIMIT=0; // 1000 (means approx. 1kb)
  std::string AWSCache::DELIMITER_FOLDER_ENTRIES=",";

  AWSCache::AWSCache(std::string bucketname):
     theBucketname(bucketname)
  {
    if (!(theServers= getenv("MEMCACHED_SERVERS")))
    {
      std::cerr << "Unable to use memcached client functionality. Please specify the MEMCACHED_SERVERS environment variable" << std::endl;
      exit(4);
    }
  }

  AWSCache::~AWSCache(){
  }

  memcached_st *
  AWSCache::get_Memcached_struct()
  {
    S3CACHE_LOG(S3CACHE_DEBUG,"AWSCache::get_Memcached_struct()","create struct");

    memcached_server_st *servers;
    memcached_st * memc=memcached_create(NULL);
    servers= memcached_servers_parse(theServers);

    // tell memc where the memcached servers are
    memcached_server_push(memc, servers);
    memcached_server_list_free(servers);

    return memc;
  }


  void AWSCache::free_Memcached_struct(memcached_st * memc)
  {
    S3CACHE_LOG(S3CACHE_DEBUG,"AWSCache::free_Memcached_struct(...)","destroy struct");

    memcached_free(memc);
  }

/*
 * delete a key
 */
  void AWSCache::delete_key(const std::string& key)
  {
    memcached_st* memc=NULL;
    try{
      memc=get_Memcached_struct();
      delete_key(memc,key);
      free_Memcached_struct(memc);
    }catch(...){
      S3CACHE_LOG(S3CACHE_ERROR,"AWSCache::delete_key(...)","error invalidating key: '" << key << "'");
      if(memc)free_Memcached_struct(memc);
    }
  }


  void AWSCache::delete_key(memcached_st* memc, const std::string& key)
  {
    memcached_return rc;

    rc=memcached_delete (memc, key.c_str(), strlen(key.c_str()),(time_t)0);

#ifndef NDEBUG
    if (rc == MEMCACHED_SUCCESS){
      S3CACHE_LOG(S3CACHE_DEBUG,"AWSCache::delete_key(...)","   successfully invalidated key: '" << key << "'");
    }else{
      S3CACHE_LOG(S3CACHE_INFO,"AWSCache::delete_key(...)","[WARNING] could not delete key: '" << key << "' from cache (rc=" << (int) rc << ": "<< memcached_strerror(memc,rc) <<")");
    }
#endif
  }

/*
 * save a key
 */
  void AWSCache::save_key(memcached_st* memc, const std::string& key, const std::string& value)
  {
    memcached_return rc;

    rc=memcached_set(memc, key.c_str(), strlen(key.c_str()),value.c_str(), strlen(value.c_str()),(time_t)0, (uint32_t)0);

#ifndef NDEBUG
    if (rc == MEMCACHED_SUCCESS){
      S3CACHE_LOG(S3CACHE_DEBUG,"AWSCache::save_cache_key(...)","successfully stored key: '" << key << "' value: '" << value << "'");
    }else{
      S3CACHE_LOG(S3CACHE_INFO,"AWSCache::save_cache_key(...)","[ERROR] could not store key: '" << key << "' in cache (rc=" << (int) rc << ": "<< memcached_strerror(memc,rc) <<")");
    }
#endif
  }


  void AWSCache::save_key(const std::string& key, const std::string& value)
  {
    memcached_st* memc=NULL;
    try{
      memc=get_Memcached_struct();
      save_key(memc,key,value);
      free_Memcached_struct(memc);
    }catch(...){
      S3CACHE_LOG(S3CACHE_ERROR,"AWSCache::save_key(...)","error saving key: '" << key << "' with value: '" << value << "'");
      if(memc)free_Memcached_struct(memc);
    }
  }


/*
 * saving a file to cache
 */
  void AWSCache::save_file(memcached_st* memc, const std::string& key, std::fstream* fstream, size_t size)
  {
    memcached_return rc;
    std::auto_ptr<char> memblock(new char [size]);

    ASSERT(fstream);
    fstream->seekg(0,std::ios_base::beg);

    fstream->read(memblock.get(),size);
    ASSERT((unsigned int)fstream->gcount()==size);

    if(size==0){
      rc=memcached_set(memc, key.c_str(), strlen(key.c_str()), "", 0,(time_t)0, (uint32_t)0);
    }else{
      rc=memcached_set(memc, key.c_str(), strlen(key.c_str()), memblock.get(), size,(time_t)0, (uint32_t)0);
    }

#ifndef NDEBUG
    if (rc == MEMCACHED_SUCCESS){
      std::string lvalue(memblock.get());
      S3CACHE_LOG(S3CACHE_DEBUG,"AWSCache::save_cache_key_file(...)","   successfully stored file: '" << key << "'");// value: '"<<lvalue<<"'");
    }else{
      S3CACHE_LOG(S3CACHE_INFO,"AWSCache::save_cache_key_file(...)","    [ERROR] could not store file: '" << key << "' in cache (rc=" << (int) rc << ": "<< memcached_strerror(memc,rc) <<")");
    }
#endif
  }

void AWSCache::save_file(const std::string& key, std::fstream* fstream, size_t size)
  {
    memcached_st* memc=NULL;
    try{
      memc=get_Memcached_struct();

      // check if file type is known
      if(key.length()>3 && key.substr(key.length()-3,key.length()).compare(".xq")==0){
        save_file(memc, key, fstream, size);
      }else if(key.length()>4 && key.substr(key.length()-4,key.length()).compare(".xml")==0){
        save_file(memc, key, fstream, size);
      }else if(key.length()>4 && key.substr(key.length()-4,key.length()).compare(".txt")==0){
        save_file(memc, key, fstream, size);
      }else if(key.length()>5 && key.substr(key.length()-5,key.length()).compare(".fcgi")==0){
        save_file(memc, key, fstream, size);
      }else if(key.length()>4 && key.substr(key.length()-4,key.length()).compare(".cgi")==0){
        save_file(memc, key, fstream, size);
      }else if(key.length()>5 && key.substr(key.length()-5,key.length()).compare(".html")==0){
        save_file(memc, key, fstream, size);
      }else if(key.length()>4 && key.substr(key.length()-4,key.length()).compare(".htm")==0){
        save_file(memc, key, fstream, size);
      }else if(key.length()==9 && key.compare(".htaccess")==0){
        save_file(memc, key, fstream, size);
      }else{
        S3CACHE_LOG(S3CACHE_ERROR,"AWSCache::save_file(...)","due to an unsupported file type: not caching file: '" << key << "'");
      }
      free_Memcached_struct(memc);
    }catch(...){
      S3CACHE_LOG(S3CACHE_ERROR,"AWSCache::save_file(...)","error saving file: '" << key << "'");
      if(memc)free_Memcached_struct(memc);
    }
  }


/*
 * save a complete stat
 */
  void AWSCache::save_stat(struct stat* stbuf, const std::string& path)
  {
    // get memc
    memcached_st* memc=NULL;

    try{
       memc=get_Memcached_struct();

       std::string key=getkey(PREFIX_STAT_ATTR,path,"mode");
       save_key(memc, key, to_string(stbuf->st_mode));

       key=getkey(PREFIX_STAT_ATTR,path,"gid").c_str();
       save_key(memc, key,to_string(stbuf->st_gid));

       key=getkey(PREFIX_STAT_ATTR,path,"oid").c_str();
       save_key(memc, key,to_string(stbuf->st_uid));

       key=getkey(PREFIX_STAT_ATTR,path,"mtime").c_str();
       save_key(memc, key,time_to_string(stbuf->st_mtime));

       key=getkey(PREFIX_STAT_ATTR,path,"size").c_str();
       save_key(memc, key,to_string(stbuf->st_size));

       key=getkey(PREFIX_STAT_ATTR,path,"nlink").c_str();
       save_key(memc, key,to_string(stbuf->st_nlink));

       free_Memcached_struct(memc);
    }catch(...){
      S3CACHE_LOG(S3CACHE_ERROR,"AWSCache::save_stat(...)","error saving file stat for file: '" << path << "'");
      if(memc)free_Memcached_struct(memc);
    }
  }


/*
 * read a key
 */
  std::string AWSCache::read_key(memcached_st* memc, const std::string& key, memcached_return* rc)
  {
    uint32_t flags;
    size_t value_length;
    char* value;
    std::string lvalue="";

    value=memcached_get(memc, key.c_str(), strlen(key.c_str()),&value_length, &flags, rc);
    if (value!=NULL){
       lvalue=std::string(value);
       free(value);
    }

#ifndef NDEBUG
    std::string lkey(key);
    if (*rc == MEMCACHED_SUCCESS){
      S3CACHE_LOG(S3CACHE_DEBUG,"AWSCache::read_key(...)","successfully read cached key: '" << lkey << "' value: '" << lvalue << "'");
    }else{
      S3CACHE_LOG(S3CACHE_INFO,"AWSCache::read_key(...)","[WARNING] could not read key: '" << lkey << "' from cache (rc=" << (int) *rc << ": "<< memcached_strerror(memc,*rc) <<")");
    }
#endif

    return lvalue;
  }

  std::string AWSCache::read_key(const std::string& key, memcached_return* rc)
  {
    memcached_st* memc=NULL;
    std::string result;
    try{
      memc=get_Memcached_struct();
      result=read_key(memc, key, rc);
      free_Memcached_struct(memc);
    }catch(...){
      S3CACHE_LOG(S3CACHE_ERROR,"AWSCache::read_key(...)","error reading key: '" << key << "'");
      if(memc)free_Memcached_struct(memc);
    }
    return result;
  }



/*
 * read a cached file
 */
  void AWSCache::read_file(memcached_st* memc, const std::string& key, std::fstream* fstream, memcached_return* rc)
  {
    uint32_t flags;
    size_t value_length;
    char* value;

    ASSERT(fstream);

    value=memcached_get(memc, key.c_str(), strlen(key.c_str()),&value_length, &flags, rc);

    std::string lkey(key);
    if (*rc == MEMCACHED_SUCCESS){
      if(value!=NULL){
        (*fstream) << value;
        fstream->flush();
      }
      S3CACHE_LOG(S3CACHE_DEBUG,"AWSCache::read_file(...)","successfully read cached file: '" << lkey << "'");
    }
#ifndef NDEBUG
    else{
      S3CACHE_LOG(S3CACHE_INFO,"AWSCache::read_file(...)","[WARNING] could not read file: '" << lkey << "' from cache (rc=" << (int) *rc << ": "<< memcached_strerror(memc,*rc) <<")");
    }
#endif
  }


  void AWSCache::read_file(const std::string& key, std::fstream* fstream, memcached_return* rc)
  {
    memcached_st* memc=NULL;
    try{
      memc=get_Memcached_struct();
      read_file(memc, key, fstream, rc);
      free_Memcached_struct(memc);
    }catch(...){
      S3CACHE_LOG(S3CACHE_ERROR,"AWSCache::read_file(...)","error reading file: '" << key << "'");
      if(memc)free_Memcached_struct(memc);
    }
  }

/*
 * read a complete stat
 */
  void AWSCache::read_stat(struct stat* stbuf,const std::string& path)
  {
    // get memc
    memcached_st* memc=NULL;

    try{
       memc=get_Memcached_struct();
       memcached_return rc;

       std::string key=getkey(PREFIX_STAT_ATTR,path,"mode");
       stbuf->st_mode=atoi(read_key(memc, key, &rc).c_str());

       key=getkey(PREFIX_STAT_ATTR,path,"gid");
       stbuf->st_gid=atoi(read_key(memc, key, &rc).c_str());

       key=getkey(PREFIX_STAT_ATTR,path,"oid");
       stbuf->st_uid=atoi(read_key(memc, key, &rc).c_str());

       key=getkey(PREFIX_STAT_ATTR,path,"mtime");
       stbuf->st_mtime=AWSCache::string_to_time(read_key(memc, key, &rc).c_str());

       key=getkey(PREFIX_STAT_ATTR,path,"size");
       stbuf->st_size=atol(read_key(memc, key, &rc).c_str());

       key=getkey(PREFIX_STAT_ATTR,path,"nlink");
       stbuf->st_nlink=atol(read_key(memc, key, &rc).c_str());

       free_Memcached_struct(memc);
    }catch(...){
      S3CACHE_LOG(S3CACHE_ERROR,"AWSCache::read_stat(...)","error reading file stat for: '" << path << "'");
      if(memc)free_Memcached_struct(memc);
    }
  }

/*******************
 * MEMCACHED HELPERS
 *******************
 */
  std::string AWSCache::getkey(std::string& prefix, std::string key, std::string attr)
  {
    std::string result="";
    result.append(theBucketname);
    result.append(":");
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

  std::string AWSCache::getParentFolder(const std::string& path)
  {
    size_t pos;
    pos=path.find_last_of("/");
    if(pos==std::string::npos){
      return "";
    }else{
      return path.substr(0,pos);
    }
  }

  void AWSCache::to_vector(std::vector<std::string>& result, std::string to_split, const std::string& delimiter){
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

  mode_t AWSCache::to_int(const std::string s)
  {
    return (mode_t) atoi(s.c_str());
  }

  time_t AWSCache::string_to_time(std::string timestring){
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

  std::string AWSCache::time_to_string(time_t rawtime){
    struct tm timeinfo;
    timeinfo = *localtime ( &rawtime );
    std::stringstream converter;
    converter << (timeinfo.tm_mon+1) << "/" << timeinfo.tm_mday << "/" << timeinfo.tm_year << " "
         << timeinfo.tm_hour << ":" << timeinfo.tm_min << ":" << timeinfo.tm_sec;
    return converter.str();
  }

  template <class T>
  std::string AWSCache::to_string(T i)
  {
    std::stringstream s;
    s << i;
    return s.str();
  }

}//namespace aws

