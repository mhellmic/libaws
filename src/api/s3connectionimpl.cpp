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
                        const std::map<std::string, std::string>* aMetaDataMap,
                        long aSize)
  {
    return new PutResponse(theConnection->put(aBucketName, aKey, aData, aContentType, aMetaDataMap, aSize));
  }

  PutResponsePtr
  S3ConnectionImpl::put(const std::string& aBucketName,
                        const std::string& aKey,
                        const char* aData,
                        const std::string& aContentType,
                        long aSize,
                        const std::map<std::string, std::string>* aMetaDataMap)
  {
    return new PutResponse(theConnection->put(aBucketName, aKey, aData, aContentType, aMetaDataMap, aSize));
  }

  std::string
  S3ConnectionImpl::getQueryString(const std::string& aBucketName,
                                   const std::string& aKey,
                                   time_t aExpiration)
  {
    return theConnection->queryString(s3::S3Connection::GET, aBucketName,
                                      aKey, aExpiration);
  }

  GetResponsePtr
  S3ConnectionImpl::get(const std::string& aBucketName,
                        const std::string& aKey,
                        const std::map<std::string, std::string>* aMetaDataMap)
  {
    return new GetResponse(theConnection->get(aBucketName, aKey, aMetaDataMap));
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

  DeleteAllResponsePtr
  S3ConnectionImpl::deleteAll(const std::string& aBucketName, const std::string& aPrefix)
  {
    return new DeleteAllResponse(theConnection->deleteAll(aBucketName, aPrefix));
  }

  HeadResponsePtr
  S3ConnectionImpl::head(const std::string& aBucketName, const std::string& aKey)
  {
    return new HeadResponse(theConnection->head(aBucketName, aKey));
  }

  S3ConnectionImpl::S3ConnectionImpl(const std::string& aAccessKeyId, 
                                     const std::string& aSecretAccessKey,
                                     const std::string& aCustomHost)
  {
    theConnection = new s3::S3Connection(aAccessKeyId, aSecretAccessKey, aCustomHost);
  }

  S3ConnectionImpl::~S3ConnectionImpl() 
  {
    delete theConnection;
  }

} /* namespace aws */
