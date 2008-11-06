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
#include <iostream>

#include "curlstreambuf.h"
#include "s3/s3response.h"

namespace aws { namespace s3 {

    S3Response::S3Response()
        : Response()
    {
    }

    S3Response::~S3Response()
    {
    }

    S3ResponseError&
    S3Response::getS3ResponseError()
    {
      return theS3ResponseError;
    }

    S3ResponseError::S3ResponseError()
    { }

    S3ResponseError::S3ResponseError(const S3ResponseError& e)
      : theErrorCode(e.theErrorCode),
        theErrorMessage(e.theErrorMessage),
        theRequestId(e.theRequestId),
        theHostId(e.theHostId)
    {}

    S3Exception::ErrorCode
    S3ResponseError::parseError ( const std::string& aString )
    {
      if ( aString.compare ( "InvalidAccessKeyId" ) == 0 ) {
        return S3Exception::InvalidAccessKeyId;
      }if ( aString.compare ( "BucketAlreadyExists" ) == 0 ) {
        return S3Exception::BucketAlreadyExists;
      }else {
        return S3Exception::NoError;
      }
    }
    
    std::string 
    S3ResponseError::getErrorCode(S3Exception::ErrorCode aCode){
      switch(aCode){
        case S3Exception::InvalidAccessKeyId:
          return "InvalidAccessKeyId";
        case S3Exception::BucketAlreadyExists:
          return "BucketAlreadyExists";
        default:
          return "Not implemented the Conversion";
      }
    }


    CreateBucketResponse::CreateBucketResponse ( const std::string& aBucketName )
        : theBucketName ( aBucketName )
    {
      theIsSuccessful = true; // true by default because we don't get a response
    }

    CreateBucketResponse::~CreateBucketResponse()
    {
    }

    ListAllBucketsResponse::ListAllBucketsResponse()
        : S3Response()
    {
    }

    ListAllBucketsResponse::~ListAllBucketsResponse()
    {
    }

    void
    ListAllBucketsResponse::open() 
    {
      theIterator = theBuckets.begin();
    }

    bool
    ListAllBucketsResponse::next(Bucket& aBucket)
    {
      if (theIterator != theBuckets.end()) {
        aBucket.BucketName = (*theIterator).BucketName;
        aBucket.CreationDate = (*theIterator).CreationDate;
        ++theIterator;
        return true;
      }
      return false;
    }

    void
    ListAllBucketsResponse::close()
    {
      theIterator = theBuckets.end();
    }

    const std::string&
    ListAllBucketsResponse::getOwnerId() const
    {
      return theOwnerId;
    } 

    const std::string&
    ListAllBucketsResponse::getOwnerDisplayName() const
    {
      return theOwnerDisplayName;
    }

    ListBucketResponse::ListBucketResponse(const std::string& aBucketName, const std::string& aPrefix,
                                           const std::string& aMarker, int aMaxKeys)
        : S3Response()
    {
    }

    ListBucketResponse::~ListBucketResponse()
    {
    }

    void
    ListBucketResponse::open()
    {
      theIterator = theKeys.begin();
    }

    bool
    ListBucketResponse::next(Key& aKey)
    {
      if (theIterator != theKeys.end()) {
        aKey.KeyValue     = (*theIterator).KeyValue;
        aKey.LastModified = (*theIterator).LastModified;
        aKey.ETag         = (*theIterator).ETag;
        aKey.Length       = (*theIterator).Length;
        ++theIterator;
        return true;
      }
      return false;
    }

    bool
    ListBucketResponse::hasNext()
    {
      return theIterator != theKeys.end();
    }

    void
    ListBucketResponse::close()
    {
      theIterator = theKeys.end();
    }

    DeleteBucketResponse::DeleteBucketResponse ( const std::string& aBucketName )
        : S3Response(),
        theBucketName ( aBucketName )
    {
      theIsSuccessful = true;
    }

    DeleteBucketResponse::~DeleteBucketResponse()
    {
    }

    PutResponse::PutResponse ( const std::string& aBucketName )
        : theBucketName ( aBucketName )
    {
    }

    PutResponse::~PutResponse()
    {
    }


    GetResponse::GetResponse ( const std::string& aBucketName, const std::string& aKey )
        : theBucketName ( aBucketName ),
          theKey ( aKey ),
          theContentLength ( 0 ),
          theStreamBuffer( 0 ),
          theInputStream( 0 ),
          theIsModified(true)
    {
    }

    GetResponse::~GetResponse()
    {
      delete theInputStream;
      delete theStreamBuffer;
    }


    HeadResponse::HeadResponse ( const std::string& aBucketName )
        : theBucketName ( aBucketName ),
          theContentLength ( 0 )
    {
    }

    HeadResponse::~HeadResponse()
    {
    }


    DeleteResponse::DeleteResponse ( const std::string& aBucketName,
                                     const std::string& aKey )
        : theBucketName ( aBucketName ),
          theKey ( aKey )
    {
    }

    DeleteResponse::~DeleteResponse()
    {
    }

    DeleteAllResponse::DeleteAllResponse ( const std::string& aBucketName,
                                           const std::string& aPrefix )
        : theBucketName ( aBucketName ),
          thePrefix ( aPrefix )
    {
    }

    DeleteAllResponse::~DeleteAllResponse()
    {
    }


} } // end namespaces
