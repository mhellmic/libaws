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
#ifndef AWS_SDB_SDBCONNECTIONIMPL_H
#define AWS_SDB_SDBCONNECTIONIMPL_H

#include "common.h"
#include <libaws/sdbconnection.h>

namespace aws {

	namespace sdb {
		class SDBConnection;
	}

	class SDBConnectionImpl: public SDBConnection {
	protected:
		// only the factory can create us
		friend class AWSConnectionFactoryImpl;
		SDBConnectionImpl(const std::string& aAccessKeyId,
				const std::string& aSecretAccessKey,
        const std::string& aCustomHost);

		sdb::SDBConnection* theConnection;

	public:
		virtual ~SDBConnectionImpl();

		virtual CreateDomainResponsePtr
    createDomain(const std::string &aDomainName);

		virtual DeleteDomainResponsePtr
    deleteDomain(const std::string &aDomainName);

		virtual DomainMetadataResponsePtr
    domainMetadata(const std::string &aDomainName);

		virtual ListDomainsResponsePtr
    listDomains(int aMaxNumberOfDomains = 0,
                const std::string& aNextToken = "");

		virtual PutAttributesResponsePtr
    putAttributes(const std::string& aDomainName, const std::string& aItemName,
				          const std::vector <aws::Attribute>& attributes);

    virtual BatchPutAttributesResponsePtr
    batchPutAttributes(const std::string& aDomainName, const SDBBatch& aBatch);

		virtual DeleteAttributesResponsePtr
    deleteAttributes(const std::string& aDomainName, const std::string& aItemName,
                     const std::vector<aws::Attribute>& attributes);

		virtual GetAttributesResponsePtr
    getAttributes(const std::string& aDomainName, const std::string& aItemName,
                  const std::string& attributeName = "");

		virtual SDBQueryResponsePtr
    query(const std::string& aDomainName, const std::string& aQueryExpression,
          int aMaxNumberOfItems = 0, const std::string& aNextToken = "");

    virtual SDBQueryWithAttributesResponsePtr
    queryWithAttributes(const std::string& aDomainName, const std::string& aQueryExpression,
                        const std::vector<std::string>& aAttributeNames, int aMaxNumberOfItems = 0,
                        const std::string& aNextToken = "");
	};
} /* namespace aws */
#endif
