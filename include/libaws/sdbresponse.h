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
#ifndef AWS_SDBTemplateResponse_API_H
#define AWS_SDBTemplateResponse_API_H

#include <vector>
#include <map>
#include <string>
#include <libaws/common.h>

namespace aws {

	typedef std::pair<std::string, std::string> AttributePair;

	// forward declaration of internal response classes
	namespace sdb {
		class CreateDomainResponse;
		class DeleteDomainResponse;
		class ListDomainsResponse;
		class PutAttributesResponse;
		class DeleteAttributesResponse;
		class GetAttributesResponse;
		class SDBQueryResponse;
	} /* namespace sqs */

	class SDBResponse: public SmartObject {
	public:
		virtual ~SDBResponse() {
		}
		;
		virtual const std::string& getRequestId() const = 0;
		virtual const std::string& getBoxUsage() const = 0;
	};

	template<class T>
	class SDBTemplateResponse: public SDBResponse {
	public:
		virtual ~SDBTemplateResponse();

		virtual const std::string&
		getRequestId() const;

		virtual const std::string&
		getBoxUsage() const;

	protected:
		T* theSDBResponse;
		SDBTemplateResponse(T*);
	};

	class CreateDomainResponse: public SDBTemplateResponse<
			sdb::CreateDomainResponse> {
	public:
		virtual ~CreateDomainResponse() {
		}
		;

	protected:
		friend class SDBConnectionImpl;
		CreateDomainResponse(sdb::CreateDomainResponse*);
	};

	class DeleteDomainResponse: public SDBTemplateResponse<
			sdb::DeleteDomainResponse> {
	public:
		virtual ~DeleteDomainResponse() {
		}
		;

	protected:
		friend class SDBConnectionImpl;
		DeleteDomainResponse(sdb::DeleteDomainResponse*);
	};

	class ListDomainsResponse: public SDBTemplateResponse<
			sdb::ListDomainsResponse> {
	public:
		const std::vector<std::string>& getDomainNames();
		const std::string& getNextToken();
		virtual ~ListDomainsResponse() {
		}
		;

	protected:
		friend class SDBConnectionImpl;
		ListDomainsResponse(sdb::ListDomainsResponse*);
	};

	class PutAttributesResponse: public SDBTemplateResponse<
			sdb::PutAttributesResponse> {
	public:
		virtual ~PutAttributesResponse() {
		}
		;

	protected:
		friend class SDBConnectionImpl;
		PutAttributesResponse(sdb::PutAttributesResponse*);
	};

	class DeleteAttributesResponse: public SDBTemplateResponse<
			sdb::DeleteAttributesResponse> {
	public:
		virtual ~DeleteAttributesResponse() {
		}
		;

	protected:
		friend class SDBConnectionImpl;
		DeleteAttributesResponse(sdb::DeleteAttributesResponse*);
	};

	class GetAttributesResponse: public SDBTemplateResponse<
			sdb::GetAttributesResponse> {
	public:
		const std::vector<AttributePair>& getAttributes();
		virtual ~GetAttributesResponse() {
		}
		;

	protected:
		friend class SDBConnectionImpl;
		GetAttributesResponse(sdb::GetAttributesResponse*);
	};

	class SDBQueryResponse: public SDBTemplateResponse<sdb::SDBQueryResponse> {
	public:
		const std::vector<std::string>& getItemNames();
		const std::string& getNextToken();
		virtual ~SDBQueryResponse() {
		}
		;

	protected:
		friend class SDBConnectionImpl;
		SDBQueryResponse(sdb::SDBQueryResponse*);
	};

} /* namespace aws */
#endif
