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

#include "s3/s3response.h"

namespace aws {
  
  template <class T>
  S3Response<T>::S3Response(T* aS3Response)
    : theS3Response(aS3Response) {}

  template <class T>
  S3Response<T>::~S3Response()
  {
    delete theS3Response;
  }

  template <class T>
  const std::string&
  S3Response<T>::getRequestId() const 
  { 
    return theS3Response->getRequestId();
  }
  
  template <class T>
  const std::string&
  S3Response<T>::getETag() const
  {
    return theS3Response->getETag();
  }
  
  template <class T>
  const std::string&
  S3Response<T>::getDate() const
  {
    return theS3Response->getDate();
  }
  
  template <class T>
  const std::string&
  S3Response<T>::getAmazonId() const
  {
    return theS3Response->getAmazonId();
  }

  /**
   * CreateBucketResponse
   */
  CreateBucketResponse::CreateBucketResponse(s3::CreateBucketResponse* r)
    : S3Response<s3::CreateBucketResponse>(r) {}

  const std::string&
  CreateBucketResponse::getBucketName() const
  {
    return theS3Response->getBucketName();
  }

  const std::string&
  CreateBucketResponse::getLocation() const
  {
    return theS3Response->getLocation();
  }

  /**
   * ListAllBucketsResponse
   */
  ListAllBucketsResponse::ListAllBucketsResponse(s3::ListAllBucketsResponse* r)
    : S3Response<s3::ListAllBucketsResponse>(r) {}

  void
  ListAllBucketsResponse::open()
  {
    theS3Response->open();
  }

  bool
  ListAllBucketsResponse::next(Bucket& aBucket)
  {
    s3::ListAllBucketsResponse::Bucket lBucket;
    if (theS3Response->next(lBucket)) {
      aBucket.BucketName = lBucket.BucketName; 
      aBucket.CreationDate = lBucket.CreationDate; 
      return true;
    }
    return false;
  }

  void
  ListAllBucketsResponse::close() 
  {
    theS3Response->close();
  }

  const std::string&
  ListAllBucketsResponse::getOwnerId() const
  {
    return theS3Response->getOwnerId();
  }

  const std::string&
  ListAllBucketsResponse::getOwnerDisplayName() const
  {
    return theS3Response->getOwnerDisplayName();
  }

  /**
   * DeleteBucketResponse
   */
  DeleteBucketResponse::DeleteBucketResponse(s3::DeleteBucketResponse* r)
   : S3Response<s3::DeleteBucketResponse>(r) {}

  const std::string&
  DeleteBucketResponse::getBucketName() const
  {
    return theS3Response->getBucketName();
  }

  /**
   * ListBucketsResponse
   */
  ListBucketResponse::ListBucketResponse(s3::ListBucketResponse* r)
    : S3Response<s3::ListBucketResponse>(r) {}

  void
  ListBucketResponse::open()
  {
    theS3Response->open();
  }

  bool
  ListBucketResponse::next(Object& aObject)
  {
    s3::ListBucketResponse::Key lKey;
    if (theS3Response->next(lKey)) {
      aObject.KeyValue     = lKey.KeyValue;
      aObject.LastModified = lKey.LastModified;
      aObject.ETag         = lKey.ETag;
      aObject.Size         = lKey.Length;
      return true;
    }
    return false;
  }

  bool
  ListBucketResponse::hasNext()
  {
    return theS3Response->hasNext();
  }

  void
  ListBucketResponse::close() 
  {
    theS3Response->close();
  }

  const std::string&
  ListBucketResponse::getBucketName() const
  {
    return theS3Response->getBucketName();
  }

  const std::string&
  ListBucketResponse::getPrefix() const
  {
    return theS3Response->getPrefix();
  }

  const std::string&
  ListBucketResponse::getMarker() const
  {
    return theS3Response->getMarker();
  }

  const std::string&
  ListBucketResponse::getDelimiter() const
  {
    return theS3Response->getDelimiter();
  }

  const std::vector<std::string>&
  ListBucketResponse::getCommonPrefixes() const
  {
    return theS3Response->getCommonPrefixes();
  }

  int
  ListBucketResponse::getMaxKeys() const
  {
    return theS3Response->getMaxKeys();
  }

  bool
  ListBucketResponse::isTruncated() const
  {
    return theS3Response->isTruncated();
  }

  /**
   * PutResponse
   */
  PutResponse::PutResponse(s3::PutResponse* r)
    : S3Response<s3::PutResponse>(r) {}

  const std::string&
  PutResponse::getBucketName() const
  {
    return theS3Response->getBucketName();
  }

  /**
   * GetResponse
   */
  GetResponse::GetResponse(s3::GetResponse* r)
    : S3Response<s3::GetResponse>(r) {}

  const std::string&
  GetResponse::getKey() const
  {
    return theS3Response->getKey();
  }

  const std::string&
  GetResponse::getBucketName() const
  {
    return theS3Response->getBucketName();
  }

  std::istream&
  GetResponse::getInputStream() const
  {
    return theS3Response->getInputStream();
  }

  long long
  GetResponse::getContentLength() const
  {
    return theS3Response->getContentLength();
  }

  const std::map<std::string, std::string>&
  GetResponse::getMetaData() const
  {
    return theS3Response->getMetaData();
  }

  const std::string&
  GetResponse::getContentType() const
  {
    return theS3Response->getContentType();
  }
  
  const Time&
  GetResponse::getLastModified() const{
    return theS3Response->getLastModified();
  }
  
  bool
  GetResponse::isModified() const{
    return theS3Response->isModified();
  }
  

  /**
   * HeadResponse
   */
  HeadResponse::HeadResponse(s3::HeadResponse* r)
    : S3Response<s3::HeadResponse>(r) {}

  const std::string&
  HeadResponse::getBucketName() const
  {
    return theS3Response->getBucketName();
  }


  const std::map<std::string, std::string>&
  HeadResponse::getMetaData() const
  {
    return theS3Response->getMetaData();
  }

  long long
  HeadResponse::getContentLength() const
  {
    return theS3Response->getContentLength();
  }

  const std::string&
  HeadResponse::getContentType() const
  {
    return theS3Response->getContentType();
  }

  /**
   * DeleteResponse
   */
  DeleteResponse::DeleteResponse(s3::DeleteResponse* r)
    : S3Response<s3::DeleteResponse>(r) {}

  const std::string&
  DeleteResponse::getKey() const
  {
    return theS3Response->getKey();
  }

  const std::string&
  DeleteResponse::getBucketName() const
  {
    return theS3Response->getBucketName();
  }

  /**
   * DeleteAllResponse
   */
  DeleteAllResponse::DeleteAllResponse(s3::DeleteAllResponse* r)
    : S3Response<s3::DeleteAllResponse>(r) {}

  const std::string&
  DeleteAllResponse::getPrefix() const
  {
    return theS3Response->getPrefix();
  }

  const std::string&
  DeleteAllResponse::getBucketName() const
  {
    return theS3Response->getBucketName();
  }

} /* namespace aws */

