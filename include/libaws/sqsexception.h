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
#ifndef AWS_SQSEXCEPTION_API_H
#define AWS_SQSEXCEPTION_API_H

#include <libaws/common.h>
#include <libaws/exception.h>

namespace aws {

	class QueryErrorResponse;

	namespace sqs {
		class SQSConnection;
	}

	class SQSException : public AWSException
	{
	public:
    
    enum ErrorCode{
      //Begin Header errors
      AccessDenied,
      AuthFailure,
      AWS_SimpleQueueService_InternalError,
      AWS_SimpleQueueService_NonExistentQueue,
      AWS_SimpleQueueService_QueueDeletedRecently,
      AWS_SimpleQueueService_QueueNameExists,
      ConflictingQueryParameter,
      ElementNotSigned,
      InternalError,
      InvalidAccessKeyId,
      InvalidAction,
      InvalidAddress,
      InvalidHttpRequest,
      InvalidParameterCombination,
      InvalidParameterValue,
      InvalidQueryParameter,
      InvalidRequest,
      InvalidSecurity,
      InvalidSecurityToken,
      MalformedSOAPSignature,
      MalformedVersion,
      MissingClientTokenId,
      MissingCredentials,
      MissingParameter,
      NoSuchVersion,
      NotAuthorizedToUseVersion,
      RequestExpired,
      RequestThrottled,
      ServiceUnavailable,
      SOAP11IncorrectDateFormat,
      SOAP11MissingAction,
      SoapBodyMissing,
      SoapEnvelopeMissing ,
      SoapEnvelopeParseError,
      UnknownEnvelopeNamespace,
      WSSecurityCorruptSignedInfo,
      WSSecurityCreatedDateIncorrectFormat,
      WSSecurityEncodingTypeError,
      WSSecurityExpiresDateIncorrectFormat,
      WSSecurityIncorrectValuetype,
      WSSecurityMissingValuetype,
      WSSecurityMultipleCredentialError,
      WSSecurityMultipleX509Error,
      WSSecuritySignatureError,
      WSSecuritySignatureMissing,
      WSSecuritySignedInfoMissing,
      WSSecurityTimestampExpired,
      WSSecurityTimestampExpiresMissing,
      WSSecurityTimestampMissing,
      WSSecurityX509CertCredentialError ,
      X509ParseError,
      //End Header errors
    
      //Receive Errors
      ReadCountOutOfRange,
      
      //Unknown
      Unknown
    
    };

		virtual const std::string& getErrorMessage() { return theErrorMessage; }
    
    virtual ErrorCode getErrorCode() { return theErrorCode; }
    
    virtual const std::string& getOrigErrorCode() { return theOrigErrorCode; }
    
    virtual const std::string& getRequestId() { return theRequestId; }
    
    static ErrorCode parseError ( const std::string& aString );

		virtual const char* what() const throw();

		virtual ~SQSException() throw();

	protected:
		SQSException(const QueryErrorResponse&);

		std::string theErrorMessage;
    
    std::string theOrigErrorCode;
    
    ErrorCode theErrorCode;
    
    std::string theRequestId;
	};

	class CreateQueueException : public SQSException
	{
		public:
			virtual ~CreateQueueException() throw();
		private:
			friend class sqs::SQSConnection;
			CreateQueueException(const QueryErrorResponse&);
	};

	class DeleteQueueException : public SQSException
	{
	public:
		virtual ~DeleteQueueException() throw();
	private:
		friend class sqs::SQSConnection;
		DeleteQueueException(const QueryErrorResponse&);
	};

	class ListQueuesException : public SQSException
	{
	public:
		virtual ~ListQueuesException() throw();
	private:
		friend class sqs::SQSConnection;
		ListQueuesException(const QueryErrorResponse&);
	};

	class SendMessageException : public SQSException
	{
	public:
		virtual ~SendMessageException() throw();
    SendMessageException(const QueryErrorResponse&);
	private:
		friend class sqs::SQSConnection;
		
	};

	class ReceiveMessageException : public SQSException
	{
	public:
		virtual ~ReceiveMessageException() throw();
	private:
		friend class sqs::SQSConnection;
		ReceiveMessageException(const QueryErrorResponse&);
	};

	class DeleteMessageException : public SQSException
	{
	public:
		virtual ~DeleteMessageException() throw();
    DeleteMessageException(const QueryErrorResponse&);
	private:
		friend class sqs::SQSConnection;
		
	};
} /* namespace aws */

#endif
