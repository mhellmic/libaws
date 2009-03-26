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

#include "sdb/sdbresponse.h"

using namespace aws;

namespace aws {
	namespace sdb {

		void SDBResponse::setBoxUsage(const std::string& aUsageStr) {
			std::istringstream i(aUsageStr);
			i >> theBoxUsage;
		}

		void ListDomainsResponse::open() {
			theIter = theDomainNames.begin();
		}

		bool ListDomainsResponse::next(std::string& aDomainName) {
			if (theIter != theDomainNames.end()) {
				aDomainName = *theIter++;
				return true;
			}
			return false;
		}

		void ListDomainsResponse::close() {
			theIter = theDomainNames.begin();
		}

		void GetAttributesResponse::open() {
			theIter = theAttributes.begin();
		}

		bool GetAttributesResponse::next(AttributePair& attribute) {
			if (theIter != theAttributes.end()) {
				attribute = *theIter++;
				return true;
			}
			return false;
		}

		void GetAttributesResponse::close() {
			theIter = theAttributes.begin();
		}

		void SDBQueryResponse::open() {
			theIter = theItemNames.begin();
		}

		bool SDBQueryResponse::next(std::string& aItemName) {
			if (theIter != theItemNames.end()) {
				aItemName = *theIter++;
				return true;
			}
			return false;
		}

		void SDBQueryResponse::close() {
			theIter = theItemNames.begin();
		}

		void SDBQueryWithAttributesResponse::open() {
			theIter = theResponseElements.begin();
		}

		bool SDBQueryWithAttributesResponse::next(ResponseElement& aResponseElement) {
			if (theIter != theResponseElements.end()) {
				aResponseElement = *theIter++;
				return true;
			}
			return false;
		}

		void SDBQueryWithAttributesResponse::close() {
			theIter = theResponseElements.begin();
		}

	}
}//namespaces
