#ifndef AWS_S3_S3CONNECTIONIMPL_H
#define AWS_S3_S3CONNECTIONIMPL_H

#include "common.h"
#include <libaws/s3connection.h>

namespace aws {

  namespace s3 {
    class S3Connection;
  }

  class S3ConnectionImpl : public S3Connection
  {
    public:
      virtual ~S3ConnectionImpl();

      CreateBucketResponsePtr
      createBucket(const std::string& aBucketName);

      ListAllBucketsResponsePtr
      listAllBuckets();

      DeleteBucketResponsePtr
      deleteBucket(const std::string& aBucketName);

      ListBucketResponsePtr
      listBucket(const std::string& aBucketName, const std::string& aPrefix, 
                 const std::string& aMarker, int aMaxKeys);

      ListBucketResponsePtr
      listBucket(const std::string& aBucketName, const std::string& aPrefix, 
                 const std::string& aMarker, const std::string& aDelimiter, int aMaxKeys);

      PutResponsePtr
      put(const std::string& aBucketName,
          const std::string& aKey,
          std::istream& aData,
          const std::string& aContentType,
          long aSize);

      PutResponsePtr
      put(const std::string& aBucketName,
          const std::string& aKey,
          const char* aData,
          const std::string& aContentType,
          long aSize);

      GetResponsePtr
      get(const std::string& aBucketName, const std::string& aKey);

      GetResponsePtr
      get(const std::string& aBucketName, const std::string& aKey, const std::string& aOldEtag);

      DeleteResponsePtr
      del(const std::string& aBucketName, const std::string& aKey);

      HeadResponsePtr
      head(const std::string& aBucketName, const std::string& aKey);

    protected:
      // only the factory can create us
      friend class AWSConnectionFactoryImpl;
      S3ConnectionImpl(const std::string& aAccessKeyId, const std::string& aSecretAccessKey);

      s3::S3Connection* theConnection;
  }; /* class S3ConnectionImpl */
} /* namespace aws */
#endif
