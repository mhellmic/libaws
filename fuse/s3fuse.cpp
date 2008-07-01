/* For more information, see http://www.macdevcenter.com/pub/a/mac/2007/03/06/macfuse-new-frontiers-in-file-systems.html. */ 
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <iostream>
#include <vector>

#define _FILE_OFFSET_BITS 64
#define FUSE_USE_VERSION  26
#include <fuse.h>
#include <libaws/aws.h>

using namespace aws;

static const char  *file_path      = "/s3.txt";
static const char   file_content[] = "S3 World!\n";
static const size_t file_size      = sizeof(file_content)/sizeof(char) - 1;

static AWSConnectionFactory* theFactory;
static char* theAccessKeyId;
static char* theSecretAccessKey;

static int
s3_getattr(const char *path, struct stat *stbuf)
{
    memset(stbuf, 0, sizeof(struct stat));

    if (strcmp(path, "/") == 0) { /* The root directory of our file system. */
        stbuf->st_mode = S_IFDIR | 0755;
        stbuf->st_nlink = 3;
    } else if (strcmp(path, file_path) == 0) { /* The only file we have. */
        stbuf->st_mode = S_IFREG | 0444;
        stbuf->st_nlink = 1;
        stbuf->st_size = file_size;
    } else { /* We reject everything else. */
        return -ENOENT;
    }

    return 0;
}

//static int
//s3_open(const char *path, struct fuse_file_info *fi)
//{
//    if (strcmp(path, file_path) != 0) { /* We only recognize one file. */
//        return -ENOENT;
//    }
//
//    if ((fi->flags & O_ACCMODE) != O_RDONLY) { /* Only reading allowed. */
//        return -EACCES;
//    }
//
//    return 0;
//}

static int
s3_readdir(const char *path,
           void *buf,
           fuse_fill_dir_t filler,
           off_t offset,
           struct fuse_file_info *fi)
{
    S3ConnectionPtr lConnection = theFactory->createS3Connection(theAccessKeyId, theSecretAccessKey);

    filler(buf, ".", NULL, 0);           /* Current directory (.)  */
    filler(buf, "..", NULL, 0);          /* Parent directory (..)  */
    try {
      ListBucketResponsePtr lRes = lConnection->listBucket("msb", path, "/", "/", -1);
      lRes->open();
      ListBucketResponse::Object o;
      while (lRes->next(o)) {
        filler(buf, o.KeyValue.c_str(), NULL, 0);
      }
      lRes->close();
      std::vector<std::string> lCommonPrefixes = lRes->getCommonPrefixes();
      for (std::vector<std::string>::const_iterator lIter = lCommonPrefixes.begin();
           lIter != lCommonPrefixes.end(); ++lIter) {
        filler(buf, (*lIter).c_str(), NULL, 0);
      }
    } catch (ListBucketException &e) {
      std::cerr << e.what() << std::endl;
    }


    return 0;
}

//static int
//s3_read(const char *path, char *buf, size_t size, off_t offset,
//           struct fuse_file_info *fi)
//{
//    if (strcmp(path, file_path) != 0) {
//        return -ENOENT;
//    }
//
//    if (offset >= file_size) { /* Trying to read past the end of file. */
//        return 0;
//    }
//
//    if (offset + size > file_size) { /* Trim the read to the file size. */
//        size = file_size - offset;
//    }
//
//    memcpy(buf, file_content + offset, size); /* Provide the content. */
//
//    return size;
//}

static struct fuse_operations s3_filesystem_operations;
//    .open    = s3_open,    /* To enforce read-only access.       */
//    .read    = s3_read,    /* To provide file content.           */


int
main(int argc, char **argv)
{
  s3_filesystem_operations.readdir = s3_readdir;
  s3_filesystem_operations.getattr = s3_getattr;

  theFactory = AWSConnectionFactory::getInstance();

  theAccessKeyId = getenv("AWS_ACCESS_KEY");
  theSecretAccessKey = getenv("AWS_SECRET_ACCESS_KEY");

  return fuse_main(argc, argv, &s3_filesystem_operations, NULL);
}
