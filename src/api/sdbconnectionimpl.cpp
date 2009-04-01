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

#include "sdb/sdbconnection.h"
#include "api/sdbconnectionimpl.h"

namespace aws {

	SDBConnectionImpl::SDBConnectionImpl(const std::string& aAccessKeyId,
			const std::string& aSecretAccessKey,
      const std::string& aCustomHost)
  {
		theConnection = new sdb::SDBConnection(aAccessKeyId, aSecretAccessKey, aCustomHost);
	}

	SDBConnectionImpl::~SDBConnectionImpl()
  {
		delete theConnection;
	}

	CreateDomainResponsePtr
  SDBConnectionImpl::createDomain(const std::string &aDomainName)
  {
		return new CreateDomainResponse(theConnection->createDomain(aDomainName));
	}

	DeleteDomainResponsePtr
  SDBConnectionImpl::deleteDomain(const std::string &aDomainName)
  {
		return new DeleteDomainResponse(theConnection->deleteDomain(aDomainName));
	}

	DomainMetadataResponsePtr
  SDBConnectionImpl::domainMetadata(const std::string &aDomainName)
  {
		return new DomainMetadataResponse(theConnection->domainMetadata(aDomainName));
	}

	ListDomainsResponsePtr
  SDBConnectionImpl::listDomains(int aMaxNumberOfDomains, const std::string& aNextToken)
  {
		return new ListDomainsResponse(theConnection->listDomains(aMaxNumberOfDomains, aNextToken));
	}

	PutAttributesResponsePtr
  SDBConnectionImpl::putAttributes(const std::string& aDomainName, const std::string& aItemName,
                                   const std::vector<aws::Attribute>& attributes)
  {
		return new PutAttributesResponse(theConnection->putAttributes(aDomainName, aItemName,
        attributes));
	}

  BatchPutAttributesResponsePtr
  SDBConnectionImpl::batchPutAttributes(const std::string& aDomainName, const SDBBatch& aBatch)
  {
    return new BatchPutAttributesResponse(theConnection->batchPutAttributes(aDomainName, aBatch));
  }

	DeleteAttributesResponsePtr
  SDBConnectionImpl::deleteAttributes(const std::string& aDomainName, const std::string& aItemName,
                                      const std::vector<aws::Attribute>& attributes)
  {
		return new DeleteAttributesResponse(theConnection->deleteAttributes(
				aDomainName, aItemName, attributes));
	}

	GetAttributesResponsePtr
  SDBConnectionImpl::getAttributes(const std::string& aDomainName, const std::string& aItemName,
                                   const std::string& attributeName)
  {
		return new GetAttributesResponse(theConnection->getAttributes(aDomainName,
				aItemName, attributeName));
	}

	SDBQueryResponsePtr
  SDBConnectionImpl::query(const std::string& aDomainName, const std::string& aQueryExpression,
                           int aMaxNumberOfItems, const std::string& aNextToken)
  {
		return new SDBQueryResponse(theConnection->query(aDomainName,
				aQueryExpression, aMaxNumberOfItems, aNextToken));
	} 

  SDBQueryWithAttributesResponsePtr
  SDBConnectionImpl::queryWithAttributes(const std::string& aDomainName,
                                         const std::string& aQueryExpression,
                                         const std::vector<std::string>& aAttributeNames,
                                         int aMaxNumberOfItems,
                                         const std::string& aNextToken)
  {
    return new SDBQueryWithAttributesResponse(theConnection->queryWithAttributes(aDomainName,
        aQueryExpression, aAttributeNames, aMaxNumberOfItems, aNextToken));
  }

}//namespace aws
