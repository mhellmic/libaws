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
#ifndef AWS_S3EXCEPTION_API_H
#define AWS_S3EXCEPTION_API_H

#include <libaws/common.h>
#include <libaws/exception.h>

namespace aws {

    namespace s3 {
      class S3Connection;
      class S3ResponseError;
    }

    class S3Exception : public AWSException
    {
    public:
      enum ErrorCode {
        AccessDenied,
        AccountProblem,
        AllAccessDisabled,
        AmbiguousGrantByEmailAddress,
        BadDigest,
        BucketAlreadyExists,
        BucketNotEmpty,
        CredentialsNotSupported,
        EntityTooLarge,
        InlineDataTooLarge,
        IncompleteBody,
        InternalError,
        InvalidAccessKeyId,
        InvalidAddressingHeader,
        InvalidArgument,
        InvalidBucketName,
        InvalidDigest,
        InvalidRange,
        InvalidSecurity,
        InvalidSOAPRequest,
        InvalidStorageClass,
        InvalidTargetBucketForLogging,
        KeyTooLong,
        InvalidURI,
        MalformedACLError,
        MalformedXMLError,
        MaxMessageLengthExceeded,
        MetadataTooLarge,
        MethodNotAllowed,
        MissingAttachment,
        MissingContentLength,
        MissingSecurityElement,
        MissingSecurityHeader,
        NoLoggingStatusForKey,
        NoSuchBucket,
        NoSuchKey,
        NotImplemented,
        NotSignedUp,
        OperationAborted,
        PreconditionFailed,
        RequestTimeout,
        RequestTimeTooSkewed,
        RequestTorrentOfBucketError,
        SignatureDoesNotMatch,
        TooManyBuckets,
        UnexpectedContent,
        UnresolvableGrantByEmailAddress,
        NoError
      };

      ErrorCode getErrorCode()             { return theErrorCode;    }
      const std::string& getErrorMessage() { return theErrorMessage; } 
      const std::string& getRequestId()    { return theRequestId;    }
      const std::string& getHostId()       { return theHostId;       }

      virtual const char* what() const throw();

      virtual ~S3Exception() throw();

      S3Exception(const ErrorCode&   theErrorCode,
                  const std::string&  theErrorMessage,
                  const std::string&  theRequestId,
                  const std::string&  theHostId);

    protected:
      S3Exception(const s3::S3ResponseError&);

      ErrorCode   theErrorCode;
      std::string theErrorMessage;
      std::string theRequestId;
      std::string theHostId;
    };

    class CreateBucketException : public S3Exception 
    {
      public:
        virtual ~CreateBucketException() throw();
      private:
        friend class s3::S3Connection;
        CreateBucketException(const s3::S3ResponseError&);
    };

    class DeleteBucketException : public S3Exception 
    {
    public:
      virtual ~DeleteBucketException() throw();
    private:
      friend class s3::S3Connection;
      DeleteBucketException(const s3::S3ResponseError&);
    };

    class ListAllBucketsException : public S3Exception 
    {
    public:
      virtual ~ListAllBucketsException() throw();
    private:
      friend class s3::S3Connection;
      ListAllBucketsException(const s3::S3ResponseError&);
    };

    class ListBucketException : public S3Exception
    {
    public:
      virtual ~ListBucketException() throw();
    private:
      friend class s3::S3Connection;
      ListBucketException(const s3::S3ResponseError&);
    };

    class GetException : public S3Exception 
    {
    public:
      virtual ~GetException() throw();
    private:
      friend class s3::S3Connection;
      GetException(const s3::S3ResponseError&);
    };

    class PutException : public S3Exception 
    {
    public:
      virtual ~PutException() throw();
    private:
      friend class s3::S3Connection;
      PutException(const s3::S3ResponseError&);
    };

    class HeadException : public S3Exception 
    {
    public:
      virtual ~HeadException() throw();
    private:
      friend class s3::S3Connection;
      HeadException(const s3::S3ResponseError&);
    };

    class DeleteException : public S3Exception 
    {
    public:
      virtual ~DeleteException() throw();
    private:
      friend class s3::S3Connection;
      DeleteException(const s3::S3ResponseError&);
    };

    class DeleteAllException : public S3Exception 
    {
    public:
      virtual ~DeleteAllException() throw();
    private:
      friend class s3::S3Connection;
      DeleteAllException(const s3::S3ResponseError&);
      DeleteAllException(const ErrorCode&   theErrorCode,
                         const std::string&  theErrorMessage,
                         const std::string&  theRequestId,
                         const std::string&  theHostId);
    };
} /* namespace aws */

#endif
