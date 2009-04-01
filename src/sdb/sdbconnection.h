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
#ifndef AWS_SDB_SDBCONNECTION_H
#define AWS_SDB_SDBCONNECTION_H

#include "common.h"

#include <map>
#include <vector>
#include <iostream>
#include <libaws/sdbconnection.h>
#include "awsqueryconnection.h"

namespace aws {

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

		class SDBConnection: public AWSQueryConnection {

		public:
			static const std::string DEFAULT_VERSION;
			static const std::string DEFAULT_HOST;

			SDBConnection(const std::string& aAccessKeyId,
					const std::string& aSecretAccessKey,
          const std::string& aCustomHost);

			CreateDomainResponse*
			createDomain(const std::string& aQueueName);

			DeleteDomainResponse*
			deleteDomain(const std::string& aDomainName);

			DomainMetadataResponse*
			domainMetadata(const std::string& aDomainName);

			ListDomainsResponse*
			listDomains(int aMaxNumberOfDomains = 0, const std::string& aNextToken = "");

			PutAttributesResponse*
			putAttributes(const std::string& aDomainName,
					const std::string& aItemName,
					const std::vector<aws::Attribute>& attributes);

      BatchPutAttributesResponse*
      batchPutAttributes(const std::string& aDomainName,
                         const aws::SDBBatch& aBatch);

			DeleteAttributesResponse*
			deleteAttributes(const std::string& aDomainName,
					const std::string& aItemName,
					const std::vector<aws::Attribute>& attributes);

			GetAttributesResponse*
			getAttributes(const std::string& aDomainName,
					const std::string& aItemName, const std::string& attributeName = "");

			SDBQueryResponse*
			query(const std::string& aDomainName,
					const std::string& aQueryExpression, int aMaxNumberOfItems = 0,
					const std::string& aNextToken = "");

      SDBQueryWithAttributesResponse*
      queryWithAttributes(const std::string& aDomainName,
                          const std::string& aQueryExpression,
                          const std::vector<std::string>& aAttributeNames,
                          int aMaxNumberOfItems,
                          const std::string& aNextToken);

		private:
			void insertAttParameter(ParameterMap& aMap,
					const std::vector<aws::Attribute>& attributes,
					bool insertReplaces);

      void insertBatchParameter(ParameterMap& aMap,
          const SDBBatch& aBatch);
		};

	} /* namespace sdb  */
} /* namespace aws */

#endif
