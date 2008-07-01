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
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sys/stat.h>

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION  26
#include <fuse.h>
#include <libaws/aws.h>

using namespace aws;


static AWSConnectionFactory* theFactory;
static char* theAccessKeyId;
static char* theSecretAccessKey;

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
      std::cerr << s3Exception.what() << std::endl; \
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


/**
 * File open operation.
 * Open should check if the operation is permitted for the given flags. 
 */
static int
s3_open(const char *path, struct fuse_file_info *fi)
{
#ifndef NDEBUG
  std::cerr << "s3_open; path: " << path << std::endl;
#endif

  S3ConnectionPtr lCon = getConnection();

  HeadResponsePtr lRes;
  S3FS_TRY
    lRes = lCon->head("msb", path);
  S3FS_CATCH(Head)
  S3FS_EXIT(0);
}

static int
s3_readdir(const char *path,
           void *buf,
           fuse_fill_dir_t filler,
           off_t offset,
           struct fuse_file_info *fi)
{
  fprintf(stderr, "readdir path: '%s'\n", path);

  S3ConnectionPtr lCon = getConnection();

  filler(buf, ".", NULL, 0);           /* Current directory (.)  */
  filler(buf, "..", NULL, 0);          /* Parent directory (..)  */

  // TODO handle full buffer by remember the marker
  // TODO provide metadata
  // TODO catch connection exception
  // TODO return error code if an exception is thrown

  ListBucketResponsePtr lRes;
  try {
    std::string lMarker;
    do {
      lRes = lCon->listBucket("msb", path, lMarker, "/", -1);
      lRes->open();
      ListBucketResponse::Object o;
      while (lRes->next(o)) {
        struct stat lStat;
        lStat.st_mode |= S_IFREG;
        lStat.st_size = o.Size;
        std::string lTmp = o.KeyValue.replace(0, strlen(path), "");
        filler(buf, lTmp.c_str(), &lStat, 0);
        lMarker = o.KeyValue;
      }
      lRes->close();
    } while (lRes->isTruncated());

    std::vector<std::string> lCommonPrefixes = lRes->getCommonPrefixes();
    for (std::vector<std::string>::const_iterator lIter = lCommonPrefixes.begin();
         lIter != lCommonPrefixes.end(); ++lIter) {
      fprintf(stderr, "common prefix: '%s'\n", (*lIter).c_str());
      std::string lTmp = (*lIter);
      lTmp.replace(0, strlen(path), "");
      lTmp.erase(lTmp.length()-1);
      struct stat lStat;
      lStat.st_mode |= S_IFDIR | S_IFREG;
      filler(buf, lTmp.c_str(), &lStat, 0);
    }
  } catch (ListBucketException &e) {
    std::cerr << e.what() << std::endl;
    S3FS_EXIT(-ENOENT);
  }

  S3FS_EXIT(0);
}

static int
s3_getattr(const char *path, struct stat *stbuf)
{
  fprintf(stderr, "getattr path: '%s'\n", path);

  S3ConnectionPtr lCon = getConnection();
  memset(stbuf, 0, sizeof(struct stat));

  HeadResponsePtr lRes;
  try {
    lRes = lCon->head("msb", path);
    return 0;
  } catch (HeadException &e) {
    std::cerr << e.what() << std::endl;
    S3FS_EXIT(-ENOENT);
  }

    if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 3;
    } else if (strcmp(path, "/test1") == 0) { /* The only file we have. */
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
    } else if (strcmp(path, "/tmp") == 0) {
        stbuf->st_mode = S_IFDIR | 0777;
        stbuf->st_nlink = 1;
    } else { /* We reject everything else. */
      S3FS_EXIT(-ENOENT);
    }

    S3FS_EXIT(0);
}

static bool test123 = false;

static int
s3_read(const char *path, char *buf, size_t size, off_t offset,
        struct fuse_file_info *fi)
{
  fprintf(stderr, "read path: '%s'\n", path);

  S3ConnectionPtr lCon = getConnection();
  memcpy(buf, "test\n", 5); /* Provide the content. */

  if (!test123) {
    GetResponsePtr lRes;
    try {
      lRes = lCon->get("msb", path);
    } catch (GetException &e) {
      std::cerr << e.what() << std::endl;
      S3FS_EXIT(-ENOENT);
    }

    test123 = true;
    S3FS_EXIT(5);
  } else {
    releaseConnection(lCon);
    S3FS_EXIT(0);
  }
}

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




static struct fuse_operations s3_filesystem_operations;

int
main(int argc, char **argv)
{
  // set callback functions
  s3_filesystem_operations.readdir    = s3_readdir;
  s3_filesystem_operations.getattr    = s3_getattr;
  s3_filesystem_operations.open       = s3_open;
  s3_filesystem_operations.read       = s3_read;
  s3_filesystem_operations.release    = s3_release;
  s3_filesystem_operations.getattr    = s3_getattr;
  s3_filesystem_operations.opendir    = s3_opendir;


  // initialization
  theFactory = AWSConnectionFactory::getInstance();

  theAccessKeyId = getenv("AWS_ACCESS_KEY");
  theSecretAccessKey = getenv("AWS_SECRET_ACCESS_KEY");

  // let's get started
  return fuse_main(argc, argv, &s3_filesystem_operations, NULL);
}
