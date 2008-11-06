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
#ifndef AWS_S3RESPONSE_API_H
#define AWS_S3RESPONSE_API_H

#include <vector>
#include <map>
#include <string>
#include <libaws/common.h>
#include <libaws/awstime.h>

namespace aws {

  // forward declaration of internal response classes
  namespace s3 {
      class CreateBucketResponse;
      class ListAllBucketsResponse;
      class DeleteBucketResponse;
      class ListBucketResponse;
      class PutResponse;
      class GetResponse;
      class HeadResponse;
      class DeleteResponse;
      class DeleteAllResponse;
  } /* namespace s3 */

  /** \brief S3Response is the base class of all classes that can be
   *         returned as a result of a S3 request.
   */
  template <class T>
  class S3Response : public SmartObject
  {
    public:
      virtual ~S3Response();

      virtual const std::string&
      getRequestId() const;
      
      virtual const std::string&
      getETag() const;
      
      virtual const std::string&
      getDate() const;
      
      virtual const std::string&
      getAmazonId() const;

      virtual T*
      get() const { return theS3Response; }

    protected:
      T* theS3Response;
      S3Response(T*);

  }; /* class S3Response */

  class CreateBucketResponse : public S3Response<s3::CreateBucketResponse>
  {
    public:
      virtual ~CreateBucketResponse() {}

      /** \brief The name of the bucket that was created.
       */
	    virtual const std::string& 
      getBucketName() const;
      
      virtual const std::string& 
      getLocation() const;

    private:
      friend class S3ConnectionImpl;
      CreateBucketResponse(s3::CreateBucketResponse*);
  }; /* class CreateBucketResponse */

  class ListAllBucketsResponse : public S3Response<s3::ListAllBucketsResponse>
  {
    public:
      struct Bucket {
        std::string BucketName;
        std::string CreationDate;
      };

      virtual ~ListAllBucketsResponse() {}

      virtual void
      open();

      virtual bool
      next(Bucket&);

      virtual void
      close();

      virtual const std::string& 
      getOwnerId() const;

      virtual const std::string&
      getOwnerDisplayName() const;

    private:
      friend class S3ConnectionImpl;
      ListAllBucketsResponse(s3::ListAllBucketsResponse*);
  }; /* class ListAllBucketsResponse */

  class DeleteBucketResponse  : public S3Response<s3::DeleteBucketResponse>
  {
    public:
      DeleteBucketResponse(s3::DeleteBucketResponse*);
      virtual ~DeleteBucketResponse() {}

      virtual const std::string&
      getBucketName() const;
    private:
      friend class S3ConnectionImpl;
  }; /* class DeleteBucketResponse */


  class ListBucketResponse  : public S3Response<s3::ListBucketResponse>
  {
    public:
      struct Object {
        std::string KeyValue;
        std::string LastModified;
        std::string ETag;
        size_t      Size;
      };

      ListBucketResponse(s3::ListBucketResponse*);
      virtual ~ListBucketResponse() {}

      virtual void
      open();

      virtual bool
      next(Object&);

      virtual bool
      hasNext();

      virtual void
      close();

      virtual const std::string&
      getBucketName() const;

      virtual const std::string&
      getPrefix() const;

      virtual const std::string&
      getMarker() const;

      virtual const std::string&
      getDelimiter() const;

      virtual const std::vector<std::string>&
      getCommonPrefixes() const;

      virtual int
      getMaxKeys() const;

      virtual bool
      isTruncated() const;

    private:
      friend class S3ConnectionImpl;
  }; /* class ListBucketsResponse */

  class PutResponse  : public S3Response<s3::PutResponse>
  {
    public:
      virtual ~PutResponse() {}

      virtual const std::string& 
      getBucketName() const;

    private:
      friend class S3ConnectionImpl;
      PutResponse(s3::PutResponse*);
  }; /* class PutResponse */

  class GetResponse  : public S3Response<s3::GetResponse>
  {
    public:
      virtual ~GetResponse() {}

      virtual const std::string&
      getKey() const;

      virtual const std::string&
      getBucketName() const;

      virtual std::istream&
      getInputStream() const;

      virtual long long
      getContentLength() const;

      virtual const std::string&
      getContentType() const;
      
      virtual const Time&
      getLastModified() const;
      
      virtual bool
      isModified() const;

      const std::map<std::string, std::string>&
      getMetaData() const;

    private:
      friend class S3ConnectionImpl;
      GetResponse(s3::GetResponse*);
  }; /* class GetResponse */

  class HeadResponse  : public S3Response<s3::HeadResponse>
  {
    public:
      virtual ~HeadResponse() {}

      virtual const std::string&
      getBucketName() const;

      virtual const std::map<std::string, std::string>&
      getMetaData() const;

      virtual long long
      getContentLength() const;

      virtual const std::string&
      getContentType() const;

    private:
      friend class S3ConnectionImpl;
      HeadResponse(s3::HeadResponse*);
  }; /* class HeadResponse */

  class DeleteResponse  : public S3Response<s3::DeleteResponse>
  {
    public:
      virtual ~DeleteResponse() {}

      virtual const std::string&
      getKey() const;

      virtual const std::string&
      getBucketName() const;

    private:
      friend class S3ConnectionImpl;
      DeleteResponse(s3::DeleteResponse*);
  }; /* class DeleteResponse */

  class DeleteAllResponse  : public S3Response<s3::DeleteAllResponse>
  {
    public:
      virtual ~DeleteAllResponse() {}

      virtual const std::string&
      getPrefix() const;

      virtual const std::string&
      getBucketName() const;

    private:
      friend class S3ConnectionImpl;
      DeleteAllResponse(s3::DeleteAllResponse*);
  }; /* class DeleteAllResponse */

} /* namespace aws */
#endif
