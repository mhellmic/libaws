#include "common.h"
#include <libaws/s3response.h>

#include "s3/s3connection.h"
#include "api/s3connectionimpl.h"

namespace aws {

  CreateBucketResponsePtr
  S3ConnectionImpl::createBucket(const std::string& aBucketName)
  {
    return new CreateBucketResponse(theConnection->createBucket(aBucketName)); 
  }

  ListAllBucketsResponsePtr
  S3ConnectionImpl::listAllBuckets()
  {
    return new ListAllBucketsResponse(theConnection->listAllBuckets());
  }

  DeleteBucketResponsePtr
  S3ConnectionImpl::deleteBucket(const std::string& aBucketName)
  {
    return new DeleteBucketResponse(theConnection->deleteBucket(aBucketName));
  }

  ListBucketResponsePtr
  S3ConnectionImpl::listBucket(const std::string& aBucketName, const std::string& aPrefix, 
                               const std::string& aMarker, int aMaxKeys)
  {
    return new ListBucketResponse(theConnection->listBucket(aBucketName, aPrefix,
                                                            aMarker, aMaxKeys));
  }

  ListBucketResponsePtr
  S3ConnectionImpl::listBucket(const std::string& aBucketName, const std::string& aPrefix, 
                               const std::string& aMarker, const std::string& aDelimiter,
                               int aMaxKeys)
  {
    return new ListBucketResponse(theConnection->listBucket(aBucketName, aPrefix,
                                                            aMarker, aDelimiter, aMaxKeys));
  }

  PutResponsePtr
  S3ConnectionImpl::put(const std::string& aBucketName,
                        const std::string& aKey,
                        std::istream& aData,
                        const std::string& aContentType,
                        long aSize)
  {
    return new PutResponse(theConnection->put(aBucketName, aKey, aData, aContentType, aSize));
  }

  PutResponsePtr
  S3ConnectionImpl::put(const std::string& aBucketName,
                        const std::string& aKey,
                        const char* aData,
                        const std::string& aContentType,
                        long aSize)
  {
    return new PutResponse(theConnection->put(aBucketName, aKey, aData, aContentType, aSize));
  }

  GetResponsePtr
  S3ConnectionImpl::get(const std::string& aBucketName, const std::string& aKey)
  {
    return new GetResponse(theConnection->get(aBucketName, aKey));
  }

  GetResponsePtr
  S3ConnectionImpl::get(const std::string& aBucketName, const std::string& aKey,
                        const std::string& aOldEtag)
  {
    return new GetResponse(theConnection->get(aBucketName, aKey, aOldEtag));
  }

  DeleteResponsePtr
  S3ConnectionImpl::del(const std::string& aBucketName, const std::string& aKey)
  {
    return new DeleteResponse(theConnection->del(aBucketName, aKey));
  }

  HeadResponsePtr
  S3ConnectionImpl::head(const std::string& aBucketName, const std::string& aKey)
  {
    return new HeadResponse(theConnection->head(aBucketName, aKey));
  }

  S3ConnectionImpl::S3ConnectionImpl(const std::string& aAccessKeyId, 
                                     const std::string& aSecretAccessKey)
  {
    theConnection = new s3::S3Connection(aAccessKeyId, aSecretAccessKey);
  }

  S3ConnectionImpl::~S3ConnectionImpl() 
  {
    delete theConnection;
  }

} /* namespace aws */
