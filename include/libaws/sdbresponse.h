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
		class DomainMetadataResponse;
		class ListDomainsResponse;
		class PutAttributesResponse;
		class BatchPutAttributesResponse;
		class DeleteAttributesResponse;
		class GetAttributesResponse;
		class SDBQueryResponse;
		class SDBQueryWithAttributesResponse;
	} /* namespace sqs */

	class SDBResponse: public SmartObject {
	public:
		virtual ~SDBResponse() {
		}
		;
		virtual const std::string& getRequestId() const = 0;
		virtual float getBoxUsage() const = 0;
    virtual double getKBOutTransfer() const = 0;
    virtual double getKBInTransfer() const = 0;

	};

	template<class T>
	class SDBTemplateResponse: public SDBResponse {
	public:
		virtual ~SDBTemplateResponse();

		virtual const std::string&
		getRequestId() const;

		virtual float	getBoxUsage() const;
    
    virtual double getKBOutTransfer() const;
    virtual double getKBInTransfer() const;
    
    virtual T*
    get() const { return theSDBResponse; }

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

	class DomainMetadataResponse: public SDBTemplateResponse<
			sdb::DomainMetadataResponse> {
	public:
		virtual ~DomainMetadataResponse() { }

    virtual const long
    getItemCount() const;

    virtual const long
    getItemNamesSizeBytes() const;

    virtual const int
    getAttributeNameCount() const;

    virtual const long
    getAttributeNamesSizeBytes() const;

    virtual const long
    getAttributeValueCount() const;

    virtual const long
    getAttributeValuesSizeBytes() const;

    virtual const long
    getTimestamp() const;

	protected:
		friend class SDBConnectionImpl;
		DomainMetadataResponse(sdb::DomainMetadataResponse*);
	};

	class ListDomainsResponse: public SDBTemplateResponse<
			sdb::ListDomainsResponse> {
	public:
		void open();
		bool next(std::string& aDomainName);
		void close();
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

	class BatchPutAttributesResponse: public SDBTemplateResponse<
			sdb::BatchPutAttributesResponse> {
	public:
		virtual ~BatchPutAttributesResponse() {
		}
		;

	protected:
		friend class SDBConnectionImpl;
		BatchPutAttributesResponse(sdb::BatchPutAttributesResponse*);
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
		void open();
		bool next(AttributePair& attribute);
		void close();
		virtual ~GetAttributesResponse() {
		}
		;

	protected:
		friend class SDBConnectionImpl;
		GetAttributesResponse(sdb::GetAttributesResponse*);
	};

	class SDBQueryResponse: public SDBTemplateResponse<sdb::SDBQueryResponse> {
	public:
		void open();
		bool next(std::string& aItemName);
		void close();
		const std::string& getNextToken();
		virtual ~SDBQueryResponse() {}

	protected:
		friend class SDBConnectionImpl;
		SDBQueryResponse(sdb::SDBQueryResponse*);
	};

  class SDBQueryWithAttributesResponse : public SDBTemplateResponse<sdb::SDBQueryWithAttributesResponse> {
  public:
    class ResponseElement {
    public:
      std::string ItemName;
      std::vector<AttributePair> Attributes;
    };
  public:
		virtual ~SDBQueryWithAttributesResponse() {}

    void
    open();

		bool
    next(ResponseElement& aResponseElement);

		void
    close();

    const std::string&
    getNextToken();

	protected:
		friend class SDBConnectionImpl;
		SDBQueryWithAttributesResponse(sdb::SDBQueryWithAttributesResponse*);
  };

} /* namespace aws */
#endif
