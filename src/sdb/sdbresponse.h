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
#ifndef AWS_SQS_SQSRESPONSE_H
#define AWS_SQS_SQSRESPONSE_H

#include <vector>
#include <map>
#include <string>
#include <libaws/common.h>
#include <awsqueryresponse.h>

namespace aws {

	// forward declaration of internal response classes
	namespace sdb {

		typedef std::pair<std::string, std::string> AttributePair;

		class SDBResponse: public QueryResponse {
		public:
			float getBoxUsage() {
	      return theBoxUsage;
	    }
			void setBoxUsage(float aUsage) {
	      theBoxUsage = aUsage;
	    }
			void setBoxUsage(const std::string& aUsageStr);

		protected:
			float theBoxUsage;
		};

		class CreateDomainResponse: public SDBResponse {
		};

		class DeleteDomainResponse: public SDBResponse {
		};

		class ListDomainsResponse: public SDBResponse {
			friend class ListDomainsHandler;
		private:
			std::vector<std::string> theDomainNames;
			std::vector<std::string>::iterator theIter;
			std::string theNextToken;
		public:
			void open();
			bool next(std::string& aDomainName);
			void close();
			const std::string& getNextToken() {
				return theNextToken;
			}
		};

		class PutAttributesResponse: public SDBResponse {
		};

		class DeleteAttributesResponse: public SDBResponse {
		};

		class GetAttributesResponse: public SDBResponse {
			friend class GetAttributesHandler;
		private:
			std::vector<AttributePair> theAttributes;
			std::vector<AttributePair>::iterator theIter;
			std::string theTmpName;
		public:
			void open();
			bool next(AttributePair& attribute);
			void close();
		};

		class SDBQueryResponse: public SDBResponse {
			friend class QueryHandler;
		private:
			std::vector<std::string> theItemNames;
			std::vector<std::string>::iterator theIter;
			std::string theNextToken;
		public:
			void open();
			bool next(std::string& aItemName);
			void close();
			const std::string& getNextToken() {
				return theNextToken;
			}
		};

	}
}//namespaces

#endif
