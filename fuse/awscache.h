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
#ifndef AWS_S3FS_CACHE
#define AWS_S3FS_CACHE

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION  26

#include <iostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <fuse.h>
#include <string.h>
#include <stdio.h>

#include <libmemcached/memcached.h>

namespace aws { 

class AWSCache
{

private:

  char* theServers;
  std::string theBucketname;

  void free_Memcached_struct(memcached_st * memc);

  memcached_st* get_Memcached_struct();

  void delete_key(memcached_st* memc, const std::string& key);

  void save_key(memcached_st* memc, const std::string& key, const std::string& value);

  void save_file(memcached_st* memc, const std::string& key, std::fstream* fstream, size_t size);

  std::string read_key(memcached_st* memc, const std::string& key, memcached_return* rc);

  void read_file(memcached_st* memc, const std::string& key, std::fstream* fstream, memcached_return* rc);

public:

  // file size limit that is cached -> bigger files are never cached
  static unsigned int FILE_CACHING_UPPER_LIMIT;

  static std::string PREFIX_EXISTS;
  static std::string PREFIX_STAT_ATTR;
  static std::string PREFIX_DIR_LS;
  static std::string PREFIX_FILE;
  static std::string DELIMITER_FOLDER_ENTRIES;

  AWSCache(std::string bucketname);

  ~AWSCache();

  void delete_key(const std::string& key);

  void save_key(const std::string& key, const std::string& value);

  void save_file(const std::string& key, std::fstream* fstream, size_t size);

  void save_stat(struct stat* stbuf, const std::string& path);

  std::string read_key(const std::string& key, memcached_return* rc);

  void read_file(const std::string& key, std::fstream* fstream, memcached_return* rc);

  void read_stat(struct stat* stbuf, const std::string& path);

/*******************
 * MEMCACHED HELPERS
 *******************
 */

  std::string getkey(std::string& prefix, std::string key, std::string attr);

  static std::string getParentFolder(const std::string& path);

  static void to_vector(std::vector<std::string>& result, std::string to_split, const std::string& delimiter);

  static mode_t to_int(const std::string s);

  static time_t string_to_time(std::string timestring);

  static std::string time_to_string(time_t rawtime);

  template <class T>
  static std::string to_string(T i);

  };

}//namespace aws


#endif
