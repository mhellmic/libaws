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

#include <libaws/s3exception.h>
#include "s3/s3response.h"
  
namespace aws {

  S3Exception::S3Exception(const s3::S3ResponseError& aError)
  {
    theErrorCode    = aError.getErrorCode();
    theErrorMessage = aError.getErrorMessage();
    theRequestId    = aError.getRequestId();
    theHostId       = aError.getHostId();
  }

  S3Exception::S3Exception(const ErrorCode&   aErrorCode,
              const std::string& aErrorMessage,
              const std::string& aRequestId,
              const std::string& aHostId)
    : theErrorCode   (aErrorCode),
      theErrorMessage(aErrorMessage),
      theRequestId   (aRequestId),
      theHostId      (aHostId)
  { }

  S3Exception::~S3Exception() throw() {}

  const char*
  S3Exception::what() const throw()
  {
   return theErrorMessage.c_str(); 
  }

  CreateBucketException::CreateBucketException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  CreateBucketException::~CreateBucketException() throw() {}

  DeleteBucketException::DeleteBucketException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  DeleteBucketException::~DeleteBucketException() throw() {}

  ListAllBucketsException::ListAllBucketsException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  ListAllBucketsException::~ListAllBucketsException() throw() {}

  ListBucketException::ListBucketException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  ListBucketException::~ListBucketException() throw() {}

  GetException::GetException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  GetException::~GetException() throw() {}

  PutException::PutException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  PutException::~PutException() throw() {}

  HeadException::HeadException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  HeadException::~HeadException() throw() {}

  DeleteException::DeleteException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  DeleteException::~DeleteException() throw() {}

  DeleteAllException::DeleteAllException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  DeleteAllException::DeleteAllException(const ErrorCode&   aErrorCode,
                                         const std::string& aErrorMessage,
                                         const std::string& aRequestId,
                                         const std::string& aHostId)
    : S3Exception(aErrorCode, aErrorMessage, aRequestId, aHostId)
  {
  }

  DeleteAllException::~DeleteAllException() throw() {}

} /* namespace aws */
