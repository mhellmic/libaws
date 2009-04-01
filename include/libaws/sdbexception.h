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
#ifndef AWS_SDBEXCEPTION_API_H
#define AWS_SDBEXCEPTION_API_H

#include <libaws/common.h>
#include <libaws/exception.h>

namespace aws {

	class QueryErrorResponse;

	namespace sdb {
		class SDBConnection;
	}

	class SDBException : public AWSException
	{
	public:

    enum ErrorCode{
      //Begin Header errors
      AccessDenied,
      AuthFailure,
      AWS_SimpleDomainService_InternalError,
      AWS_SimpleDomainService_NonExistentDomain,
      AWS_SimpleDomainService_DomainDeletedRecently,
      AWS_SimpleDomainService_DomainNameExists,
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

		virtual ~SDBException() throw();

	protected:
		SDBException(const QueryErrorResponse&);

		std::string theErrorMessage;

    std::string theOrigErrorCode;

    ErrorCode theErrorCode;

    std::string theRequestId;
	};

	class CreateDomainException : public SDBException
	{
		public:
			virtual ~CreateDomainException() throw();
		private:
			friend class sdb::SDBConnection;
			CreateDomainException(const QueryErrorResponse&);
	};

	class DeleteDomainException : public SDBException
	{
	public:
		virtual ~DeleteDomainException() throw();
	private:
		friend class sdb::SDBConnection;
		DeleteDomainException(const QueryErrorResponse&);
	};

	class DomainMetadataException : public SDBException
	{
	public:
		virtual ~DomainMetadataException() throw();
	private:
		friend class sdb::SDBConnection;
		DomainMetadataException(const QueryErrorResponse&);
	};

	class ListDomainsException : public SDBException
	{
	public:
		virtual ~ListDomainsException() throw();
	private:
		friend class sdb::SDBConnection;
		ListDomainsException(const QueryErrorResponse&);
	};

	class PutAttributesException : public SDBException
	{
	public:
		virtual ~PutAttributesException() throw();
	private:
		friend class sdb::SDBConnection;
		PutAttributesException(const QueryErrorResponse&);
	};

  class BatchPutAttributesException : public SDBException
  {
  public:
    virtual ~BatchPutAttributesException() throw();
  private:
    friend class sdb::SDBConnection;
    BatchPutAttributesException(const QueryErrorResponse&);
  };

	class DeleteAttributesException : public SDBException
	{
	public:
		virtual ~DeleteAttributesException() throw();
	private:
		friend class sdb::SDBConnection;
		DeleteAttributesException(const QueryErrorResponse&);
	};

	class GetAttributesException : public SDBException
	{
	public:
		virtual ~GetAttributesException() throw();
	private:
		friend class sdb::SDBConnection;
		GetAttributesException(const QueryErrorResponse&);
	};

	class QueryException : public SDBException
	{
	public:
		virtual ~QueryException() throw();
	private:
		friend class sdb::SDBConnection;
		QueryException(const QueryErrorResponse&);
	};
} /* namespace aws */

#endif
