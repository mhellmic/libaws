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
                 const std::string& aMarker, const std::string& aDelimiter, int aMaxKeys);

      PutResponsePtr
      put(const std::string& aBucketName,
          const std::string& aKey,
          std::istream& aData,
          const std::string& aContentType,
          const std::map<std::string, std::string>* aMetaDataMap = 0,
          long aSize = -1);

      PutResponsePtr
      put(const std::string& aBucketName,
          const std::string& aKey,
          const char* aData,
          const std::string& aContentType,
          long aSize,
          const std::map<std::string, std::string>* aMetaDataMap = 0);

      std::string
      getQueryString(const std::string& aBucket,
                     const std::string& aKey,
                     time_t aExpiration);

      GetResponsePtr
      get(const std::string& aBucketName,
          const std::string& aKey,
          const std::map<std::string, std::string>* aMetaDataMap = 0);

      GetResponsePtr
      get(const std::string& aBucketName, const std::string& aKey, const std::string& aOldEtag);

      DeleteResponsePtr
      del(const std::string& aBucketName, const std::string& aKey);

      DeleteAllResponsePtr
      deleteAll(const std::string& aBucketName, const std::string& aPrefix);

      HeadResponsePtr
      head(const std::string& aBucketName, const std::string& aKey);

    protected:
      // only the factory can create us
      friend class AWSConnectionFactoryImpl;
      S3ConnectionImpl(const std::string& aAccessKeyId, const std::string& aSecretAccessKey,
                       const std::string& aCustomHost);

      s3::S3Connection* theConnection;
  }; /* class S3ConnectionImpl */
} /* namespace aws */
#endif
