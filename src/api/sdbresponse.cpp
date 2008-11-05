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

#include <libaws/sdbresponse.h>

#include "sdb/sdbresponse.h"

namespace aws {

	template<class T>
	SDBTemplateResponse<T>::SDBTemplateResponse(T* aResponse) :
		theSDBResponse(aResponse) {
	}

	template<class T>
	SDBTemplateResponse<T>::~SDBTemplateResponse() {
		delete theSDBResponse;
	}

	template<class T>
	const std::string&
	SDBTemplateResponse<T>::getRequestId() const {
		return theSDBResponse->getRequestId();
	}

	template<class T>
	float SDBTemplateResponse<T>::getBoxUsage() const {
		return theSDBResponse->getBoxUsage();
	}
  
  template <class T>
  double SDBTemplateResponse<T>::getKBOutTransfer() const {
    return  theSDBResponse->getOutTransfer() / 1024;
  }
  
  template <class T>
  double SDBTemplateResponse<T>::getKBInTransfer() const {
    return  ((double)theSDBResponse->getInTransfer()) / 1024;
    }

	CreateDomainResponse::CreateDomainResponse(sdb::CreateDomainResponse* r) :
		SDBTemplateResponse<sdb::CreateDomainResponse> (r) {
	}

	DeleteDomainResponse::DeleteDomainResponse(sdb::DeleteDomainResponse* r) :
		SDBTemplateResponse<sdb::DeleteDomainResponse> (r) {
	}

	ListDomainsResponse::ListDomainsResponse(sdb::ListDomainsResponse* r) :
		SDBTemplateResponse<sdb::ListDomainsResponse> (r) {
	}

	void ListDomainsResponse::open() {
		theSDBResponse->open();
	}

	bool ListDomainsResponse::next(std::string& aDomainName) {
		return theSDBResponse->next(aDomainName);
	}

	void ListDomainsResponse::close() {
		theSDBResponse->close();
	}

	const std::string& ListDomainsResponse::getNextToken() {
		return theSDBResponse->getNextToken();
	}

	PutAttributesResponse::PutAttributesResponse(sdb::PutAttributesResponse* r) :
		SDBTemplateResponse<sdb::PutAttributesResponse> (r) {
	}

	DeleteAttributesResponse::DeleteAttributesResponse(
			sdb::DeleteAttributesResponse* r) :
		SDBTemplateResponse<sdb::DeleteAttributesResponse> (r) {
	}

	GetAttributesResponse::GetAttributesResponse(sdb::GetAttributesResponse* r) :
		SDBTemplateResponse<sdb::GetAttributesResponse> (r) {
	}

	void GetAttributesResponse::open() {
		theSDBResponse->open();
	}

	bool GetAttributesResponse::next(AttributePair& attribute) {
		return theSDBResponse->next(attribute);
	}

	void GetAttributesResponse::close() {
		theSDBResponse->close();
	}

	SDBQueryResponse::SDBQueryResponse(sdb::SDBQueryResponse* r) :
		SDBTemplateResponse<sdb::SDBQueryResponse> (r) {
	}

	void SDBQueryResponse::open() {
		theSDBResponse->open();
	}

	bool SDBQueryResponse::next(std::string& aDomainName) {
		return theSDBResponse->next(aDomainName);
	}

	void SDBQueryResponse::close() {
		theSDBResponse->close();
	}

	const std::string& SDBQueryResponse::getNextToken() {
		return theSDBResponse->getNextToken();
	}

} /* namespace aws */
