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
#ifndef AWS_SDB_SDBCONNECTION_API_H
#define AWS_SDB_SDBCONNECTION_API_H

#include <istream>
#include <map>
#include <vector>
#include <libaws/common.h>

namespace aws {

  namespace sdb {
    class SDBConnection;
  }

	class Attribute {
		std::string name;
		std::string value;
		bool replace;

	public:
		Attribute(const std::string& aName, const std::string& aValue,
				bool aReplace) :
			name(aName), value(aValue), replace(aReplace) {
		}

		const std::string& getName() const {
			return name;
		}

		const std::string& getValue() const {
			return value;
		}

		bool isReplace() const {
			return replace;
		}
	};

  class SDBBatch {
    protected:
      friend class sdb::SDBConnection;
      std::map<std::string, std::vector<Attribute> > theBatch;

    public:
      void
      addItem(const std::string& aItem, const std::vector<Attribute>& aAttributes)
      {
          theBatch.insert(std::pair<std::string, std::vector<Attribute> >(aItem, aAttributes));
      }

      void
      clear() { theBatch.clear(); }

      size_t
      size() const { return theBatch.size(); }
  };

	class SDBConnection: public SmartObject {
	public:
		virtual ~SDBConnection() {
		}

		virtual CreateDomainResponsePtr
    createDomain(const std::string &aDomainName) = 0;

		virtual DeleteDomainResponsePtr
		deleteDomain(const std::string &aDomainName) = 0;

		virtual DomainMetadataResponsePtr
		domainMetadata(const std::string &aDomainName) = 0;

		virtual ListDomainsResponsePtr
    listDomains(int aMaxNumberOfDomains = 0, const std::string& aNextToken = "") = 0;

		virtual PutAttributesResponsePtr
    putAttributes(const std::string& aDomainName, const std::string& aItemName,
          				const std::vector<aws::Attribute>& attributes) = 0;

    virtual BatchPutAttributesResponsePtr
    batchPutAttributes(const std::string& aDomainName, const SDBBatch& aBatch) = 0;

		virtual DeleteAttributesResponsePtr
    deleteAttributes(const std::string& aDomainName, const std::string& aItemName,
                     const std::vector<aws::Attribute>& attributes) = 0;

		virtual GetAttributesResponsePtr
    getAttributes(const std::string& aDomainName, const std::string& aItemName,
                  const std::string& attributeName = "") = 0;

		virtual SDBQueryResponsePtr
    query(const std::string& aDomainName, const std::string& aQueryExpression,
          int aMaxNumberOfItems = 0, const std::string& aNextToken = "") = 0;

    virtual SDBQueryWithAttributesResponsePtr
    queryWithAttributes(const std::string& aDomainName, const std::string& aQueryExpression,
                        const std::vector<std::string>& aAttributeNames, int aMaxNumberOfItems = 0,
                        const std::string& aNextToken = "") = 0;

	};

}
#endif
