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

#include <libaws/sdbexception.h>
#include "sdb/sdbresponse.h"

namespace aws {

	SDBException::SDBException(const QueryErrorResponse& aError) {
		theErrorMessage = aError.getErrorMessage();
		theOrigErrorCode = aError.getErrorCode();
		theErrorCode = parseError(theOrigErrorCode);
		theRequestId = aError.getRequestId();
	}

	SDBException::~SDBException() throw() {
	}

	const char*
	SDBException::what() const throw() {
    std::stringstream lTmp;
    lTmp << "Code:" << theOrigErrorCode << " Message:" << theErrorMessage << " RequestId:" << theRequestId;
    lTmp.str().c_str();
    return lTmp.str().c_str();;
	}

	SDBException::ErrorCode SDBException::parseError(const std::string& aString) {
		return SDBException::Unknown;
	}

	CreateDomainException::CreateDomainException(const QueryErrorResponse& aError) :
		SDBException(aError) {
	}

	CreateDomainException::~CreateDomainException() throw() {
	}

	ListDomainsException::ListDomainsException(const QueryErrorResponse& aError) :
		SDBException(aError) {
	}

	ListDomainsException::~ListDomainsException() throw() {
	}

	DeleteDomainException::DeleteDomainException(const QueryErrorResponse& aError) :
		SDBException(aError) {
	}

	DeleteDomainException::~DeleteDomainException() throw() {
	}

	DomainMetadataException::DomainMetadataException(const QueryErrorResponse& aError) :
		SDBException(aError) {
	}

	DomainMetadataException::~DomainMetadataException() throw() {
	}

	PutAttributesException::PutAttributesException(const QueryErrorResponse& aError) :
		SDBException(aError) {
	}

	PutAttributesException::~PutAttributesException() throw() {
	}

	BatchPutAttributesException::BatchPutAttributesException(const QueryErrorResponse& aError) :
		SDBException(aError) {
	}

	BatchPutAttributesException::~BatchPutAttributesException() throw() {
	}

	DeleteAttributesException::DeleteAttributesException(
			const QueryErrorResponse& aError) :
		SDBException(aError) {
	}

	DeleteAttributesException::~DeleteAttributesException() throw() {
	}

	GetAttributesException::GetAttributesException(
			const QueryErrorResponse& aError) :
		SDBException(aError) {
	}

	GetAttributesException::~GetAttributesException() throw() {
	}

	QueryException::QueryException(
			const QueryErrorResponse& aError) :
		SDBException(aError) {
	}

	QueryException::~QueryException() throw() {
	}

} /* namespace aws */

