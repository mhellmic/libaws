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
#ifndef AWS_S3_S3CONNECTION_H
#define AWS_S3_S3CONNECTION_H

#include "common.h"

#include <map>
#include <iostream>

#include "awsconnection.h"

namespace aws {

  // forward declarations
  class    AWSConnectionFactory;
  class    CallingFormat;
  class    Canonizer;
  class    RequestHeaderMap;
  class    S3ConnectionImpl;
  typedef  std::map < std::string, std::string > PathArgs_t;

  namespace s3 {

    class  S3Object;
    struct S3CallBackWrapper;


    class S3Connection : public aws::AWSConnection {

      friend class    ::aws::S3ConnectionImpl;
      friend class    ::aws::Canonizer;

    private:
      //! Instance of this class are only created by the aws::AWSConnectionFactory
      S3Connection(const std::string& aAccessKeyId, const std::string& aSecretAccessKey,
                   const std::string& aCustomHost);

    protected:
      static std::string DEFAULT_HOST; // the amazon s3 default hostname

      enum ActionType {
        CREATE_BUCKET = 0,
        LIST_ALL_BUCKETS,
        LIST_BUCKET,
        DELETE_BUCKET,
        PUT,
        GET,
        DELETE,
        HEAD
      };

      unsigned int    theEncryptedResultSize;
      char*           theBase64EncodedString;
      unsigned char   theEncryptedResult[1024];

    public:
      virtual ~S3Connection();

      std::string getProtocolVersion() { return "2006-03-01"; }

      CreateBucketResponse*
      createBucket(const std::string& aBucketName);

      ListAllBucketsResponse*
      listAllBuckets();

      DeleteBucketResponse*
      deleteBucket(const std::string& aBucketName, RequestHeaderMap * aHeaderMap = 0);

      ListBucketResponse*
      listBucket(const std::string& aBucketName, const std::string& aPrefix, 
                 const std::string& aMarker, int aMaxKeys);

      ListBucketResponse*
      listBucket(const std::string& aBucketName, const std::string& aPrefix, 
                 const std::string& aMarker, const std::string& aDelimiter, int aMaxKeys);

      PutResponse*
      put(const std::string& aBucketName,
          const std::string& aKey,
          std::istream& aObject,
          const std::string& aContentType,
          const std::map<std::string, std::string>* aMetaDataMap,
          long aSize);

      PutResponse*
      put(const std::string& aBucketName,
          const std::string& aKey,
          const char* aObject, 
          const std::string& aContentType,
          const std::map<std::string, std::string>* aMetaDataMap,
          long aSize);

      std::string
      queryString(ActionType aActionType, const std::string& aBucketName,
                  const std::string& aKey, time_t aExpiration);

      GetResponse*
      get(const std::string& aBucketName, const std::string& aKey, 
          const std::string& aOldEtag);

      GetResponse*
      get(const std::string& aBucketName, const std::string& aKey, 
          const std::map<std::string, std::string>* aMetaDataMap);

      DeleteResponse*
      del(const std::string& aBucketName, const std::string& aKey);

      DeleteAllResponse*
      deleteAll(const std::string& aBucketName, const std::string& aPrefix);

      HeadResponse*
      head(const std::string& aBucketName, const std::string& aKey);

    private:
      void
      makeRequest(const std::string& aBucketName, ActionType aActionType, S3CallBackWrapper* aResponse,
                  PathArgs_t* aPathArgsMap, RequestHeaderMap* aHeaderMap);

      void
      makeRequest(const std::string& aBucketName, ActionType aActionType, S3CallBackWrapper* aResponse,
                  PathArgs_t * aPathArgsMap, RequestHeaderMap * aHeaderMap,
                  const std::string& aKey, S3Object* aObject);

      void            setRequestMethod(ActionType aActionType);

      //all the callback handlers
      static          size_t
      getS3Data(void *aBuffer, size_t aSize, size_t nmemb, void *userp);

      static          size_t
      setCreateBucketData(void *aBuffer, size_t aSize, size_t nmemb, void *stream);

      static          size_t
      setPutData(void *aBuffer, size_t aSize, size_t nmemb, void *stream);

      static          size_t
      getHeaderData(void *ptr, size_t size, size_t nmemb, void *stream);

    public:
      static std::string
      requestTypeForAction(ActionType aType);

    };

  } /* namespace s3  */
}	/* namespace aws */

#endif				/* !AWS_S3_S3CONNECTION_H */
