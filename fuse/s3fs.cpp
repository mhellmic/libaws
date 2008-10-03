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
 *   - metadata: 
 *     - dir : 1
 *     - mode_t : int
 *     - gid : int
 *     - oid : int
 * file:
 *  - object with the path/name of the file, e.g. /testdir/testfile
 *  - metadata:
 *     - file: 1
 *     - mode_t : int
 *     - gid : int
 *     - oid : int
 */
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sys/stat.h>
#include <map>
#include <sstream>

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION  26
#include <fuse.h>
#include <libaws/aws.h>

using namespace aws;


static AWSConnectionFactory* theFactory;
static char* theAccessKeyId;
static char* theSecretAccessKey;
static std::string BUCKETNAME("msb");

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
      std::cerr << s3Exception.getErrorMessage() << std::endl; \
      S3FS_EXIT(-ENOENT); \
    } catch (AWSConnectionException & conException) { \
      std::cerr << conException.what() << std::endl; \
      S3FS_EXIT(-ECONNREFUSED); \
    }
#else
#  define S3FS_CATCH(kind) \
    } catch (kind ## Exception & s3Exception) { \
      std::cerr << s3Exception.what() << std::endl; \
      S3FS_EXIT(-ENOENT); \
    } catch (AWSConnectionException & conException) { \
      std::cerr << conException.what() << std::endl; \
      S3FS_EXIT(-ECONNREFUSED); \
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
  stbuf->st_uid  |= to_int(aMap["uid"]);

  if (aMap.count("dir") != 0) {
    stbuf->st_mode |= S_IFDIR;
  } else if (aMap.count("file") != 0) {
    stbuf->st_mode |= S_IFREG;
    stbuf->st_size = 5;
  } 
  // TODO date
}

/** 
 * operation functions
 */
static int
s3_getattr(const char *path, struct stat *stbuf)
{
#ifndef NDEBUG
  std::cerr << "getattr path: " << path << std::endl;
#endif

  // we always immediately exit if the root dir is requested.
  if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
    stbuf->st_mode = S_IFDIR | 0777;
    stbuf->st_nlink = 3;
    return 0;
  }

  S3ConnectionPtr lCon = getConnection();
  S3FS_TRY
    memset(stbuf, 0, sizeof(struct stat));

    // check if we have that path
    HeadResponsePtr lRes;
    lRes = lCon->head(BUCKETNAME, path);
    map_t lMap = lRes->getMetaData();
#ifndef NDEBUG
    std::cerr << "  requested metadata for " << path << std::endl;
    for (map_t::iterator lIter = lMap.begin(); lIter != lMap.end(); ++lIter) {
      std::cerr << "    got " << (*lIter).first << " : " << (*lIter).second << std::endl;
    }
    std::cerr << "    content-length: " << lRes->getContentLength() << std::endl;
#endif

    // set the meta data in the stat struct
    fill_stat(lMap, stbuf, lRes->getContentLength());

    S3FS_EXIT(0);
  S3FS_CATCH(Head)
}

static int
s3_mkdir(const char *path, mode_t mode)
{
#ifndef NDEBUG
  std::cerr << "mkdir: " << path << std::endl;
  std::cerr << "  mode: " << mode << std::endl;
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
  std::cerr << "rmdir: " << path << std::endl;
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

static int
s3_readdir(const char *path,
           void *buf,
           fuse_fill_dir_t filler,
           off_t offset,
           struct fuse_file_info *fi)
{
#ifndef NDEBUG
  std::cerr << "readdir: " << path << std::endl;
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
      lRes = lCon->listBucket(BUCKETNAME, lPath, lMarker, "/", -1);
      lRes->open();
      ListBucketResponse::Object o;
      while (lRes->next(o)) {
        struct stat lStat;
        memset(&lStat, 0, sizeof(struct stat));

#ifndef NDEBUG
        std::cerr << "  result: " << o.KeyValue << std::endl;
#endif
        std::string lTmp = o.KeyValue.replace(0, lPath.length(), "");
        filler(buf, lTmp.c_str(), &lStat, 0);
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
  std::cerr << "create: " << path << std::endl;
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


static int
s3_read(const char *path,
        char *buf,
        size_t size,
        off_t offset,
        struct fuse_file_info *fi)
{
#ifndef NDEBUG
  std::cerr << "read: " << path << std::endl;
#endif

  S3ConnectionPtr lCon = getConnection();
  memcpy(buf, "test\n", 5); /* Provide the content. */
  return 5;
#if 0
  GetResponsePtr lRes;
  S3FS_TRY
    lRes = lCon->get(BUCKETNAME, path);
    S3FS_EXIT(0);
  S3FS_CATCH(Put)
#endif
}

#if 0
static int
s3_release(const char *path, struct fuse_file_info *fi)
{
  fprintf(stderr, "release path: '%s'\n", path);

  S3ConnectionPtr lCon = getConnection();
  S3FS_EXIT(0);
}

static int
s3_opendir(const char *path, struct fuse_file_info *fi)
{
  fprintf(stderr, "opendir: '%s'\n", path);

  S3ConnectionPtr lCon = getConnection();
  S3FS_EXIT(0);
}
#endif




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
  // can't be supported because s3 doesn't allow to change meta data without putting the object again
  s3_filesystem_operations.read       = s3_read;

  // initialization
  theFactory = AWSConnectionFactory::getInstance();

  theAccessKeyId = getenv("AWS_ACCESS_KEY");
  theSecretAccessKey = getenv("AWS_SECRET_ACCESS_KEY");

  // let's get started
  return fuse_main(argc, argv, &s3_filesystem_operations, NULL);
}
