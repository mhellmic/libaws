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
#include <iostream>
#include <sstream>
#include <vector>
#include <stdlib.h>
#include <libaws/aws.h>

using namespace aws;

void printVector(const std::vector<std::string>& strVec) {
	for (std::vector<std::string>::const_iterator iter = strVec.begin(); iter
			!= strVec.end(); iter++) {
		std::cout << *iter << std::endl;
	}
}

void printBoxUsage(const SDBResponse& resp) {
	std::cout << "RequestId: " << resp.getRequestId() << " BoxUsage:"
			<< resp.getBoxUsage() << std::endl;
}

int createDomain(SDBConnection* lCon) {
	try {
		CreateDomainResponsePtr lPtr = lCon->createDomain("testDomain");
		std::cout << "Domain created successfully." << std::endl;
		printBoxUsage(*lPtr);
	}
	catch (CreateDomainException& e) {
		std::cerr << "Couldn't create domain" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int deleteDomain(SDBConnection* lCon) {
	try {
		DeleteDomainResponsePtr lPtr = lCon->deleteDomain("testDomain");
		std::cout << "Domain deleted successfully." << std::endl;
		printBoxUsage(*lPtr);
	}
	catch (DeleteDomainException& e) {
		std::cerr << "Couldn't delete domain" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int listDomains(SDBConnection* lCon) {
	try {
		ListDomainsResponsePtr lPtr = lCon->listDomains();
		std::cout << "Listing of domains: " << std::endl;
		lPtr->open();
		std::string name;
		while (lPtr->next(name)) {
			std::cout << name << std::endl;
		}
		printBoxUsage(*lPtr);
	}
	catch (ListDomainsException& e) {
		std::cerr << "Couldn't list domains" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int putAttributes(SDBConnection* lCon) {
	try {
		std::vector<Attribute> attributes;
		attributes.push_back(Attribute("id", "id0", false));
		attributes.push_back(Attribute("amount", "5", false));
		PutAttributesResponsePtr lPtr = lCon->putAttributes("testDomain",
				"testItem", attributes);
		std::cout << "Attributes put successfully." << std::endl;
		printBoxUsage(*lPtr);
	}
	catch (PutAttributesException& e) {
		std::cerr << "Couldn't put attributes" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int deleteAttributes(SDBConnection* lCon) {
	try {
		std::vector<Attribute> attributes;
		attributes.push_back(Attribute("id", "id0", false));
		DeleteAttributesResponsePtr lPtr = lCon->deleteAttributes("testDomain",
				"testItem", attributes);
		std::cout << "Attribute 'id' deleted successfully." << std::endl;
		printBoxUsage(*lPtr);
	}
	catch (DeleteAttributesException& e) {
		std::cerr << "Couldn't delete attributes" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int getAttributes(SDBConnection* lCon) {
	try {
		GetAttributesResponsePtr lPtr = lCon->getAttributes("testDomain",
				"testItem");
		std::cout << "Get Attributes:" << std::endl;
		lPtr->open();
		AttributePair attPair;
		while (lPtr->next(attPair)) {
			std::cout << "Name: " << attPair.first << " Value: " << attPair.second
					<< std::endl;
		}
		printBoxUsage(*lPtr);
	}
	catch (GetAttributesException& e) {
		std::cerr << "Couldn't get attributes" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int query(SDBConnection* lCon) {
	try {
		std::string query("['amount'<'6']");
		SDBQueryResponsePtr lPtr = lCon->query("testDomain", query);
		std::cout << "Query result for query " << query << ": " << std::endl;
		lPtr->open();
		std::string name;
		while (lPtr->next(name)) {
			std::cout << name << std::endl;
		}
		printBoxUsage(*lPtr);
	}
	catch (QueryException& e) {
		std::cerr << "Couldn't query the query" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int queryWithAttributes(SDBConnection* lCon) {
	try {
		std::string query("['amount'<'6']");

    std::vector<std::string> lAttributes;
    lAttributes.push_back("amount");

		SDBQueryWithAttributesResponsePtr lPtr = lCon->queryWithAttributes("testDomain", query, lAttributes);
		std::cout << "Query result for queryWithAttributes " << query << ": " << std::endl;
		lPtr->open();
    SDBQueryWithAttributesResponse::ResponseElement lRes;
		while (lPtr->next(lRes)) {
			std::cout << lRes.ItemName << std::endl;
			std::cout << lRes.Attributes.size() << std::endl;
		}
		printBoxUsage(*lPtr);
	}
	catch (QueryException& e) {
		std::cerr << "Couldn't query the queryWithAttributes" << std::endl;
		std::cerr << e.what() << std::endl;
		return 1;
	}
	return 0;
}

int sdbtest(int argc, char** argv) {
	AWSConnectionFactory* lFactory = AWSConnectionFactory::getInstance();

	std::cout << "Testing libaws version " << lFactory->getVersion() << std::endl;

	char* lAccessKeyId = getenv("AWS_ACCESS_KEY");
	char* lSecretAccessKey = getenv("AWS_SECRET_ACCESS_KEY");
	if (lAccessKeyId == 0 || lSecretAccessKey == 0) {
		std::cerr
				<< "Environment variables (i.e. AWS_ACCESS_KEY or AWS_SECRET_ACCESS_KEY) not set"
				<< std::endl;
		return 1;
	}

	SDBConnectionPtr lConSmart = lFactory->createSDBConnection(lAccessKeyId,
			lSecretAccessKey);
	SDBConnection* lCon = lConSmart.get();

	try {
		if (createDomain(lCon) != 0) {
			return 1;
		}
		if (listDomains(lCon) != 0) {
			return 1;
		}
		if (putAttributes(lCon) != 0) {
			return 1;
		}
		if (getAttributes(lCon) != 0) {
			return 1;
		}
		if (deleteAttributes(lCon) != 0) {
			return 1;
		}
		if (getAttributes(lCon) != 0) {
			return 1;
		}
		if (query(lCon) != 0) {
			return 1;
		}
		if (queryWithAttributes(lCon) != 0) {
			return 1;
		}
		if (deleteDomain(lCon) != 0) {
			return 1;
		}
	}
	catch (AWSConnectionException& e) {
		std::cerr << e.what() << std::endl;
		return 2;
	}

	lFactory->shutdown();

	return 0;
}
