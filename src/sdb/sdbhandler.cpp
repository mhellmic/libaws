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
#include "sdb/sdbhandler.h"
#include "sdb/sdbresponse.h"
#include <iostream>
#include <stdlib.h>

#include <string>

using namespace aws;

namespace aws {
	namespace sdb {

		template<class T>
		void SDBHandler<T>::startElement(const xmlChar * localname,
				int nb_attributes, const xmlChar ** attributes) {
			if (xmlStrEqual(localname, (xmlChar*) "ErrorResponse") || xmlStrEqual(localname, (xmlChar*) "Error")) {
				theIsSuccessful = false;
			}
			else if (theIsSuccessful && xmlStrEqual(localname, (xmlChar*) "BoxUsage")) {
				setState(BoxUsage);
			}
			else if (theIsSuccessful) {
				responseStartElement(localname, nb_attributes, attributes);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "Code")) {
				setState(ERROR_Code);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "Message")) {
				setState(ERROR_Message);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "RequestID")) {
				setState(RequestId);
			}

		}

		template<class T>
		void SDBHandler<T>::characters(const xmlChar * value, int len) {

			if (theIsSuccessful) {
				if (isSet(BoxUsage)) {
					std::string lStrValue((const char*) value, len);
					theResponse->setBoxUsage(lStrValue);
				}
				else {
					responseCharacters(value, len);
				}
			}
			else {
				std::string lStrValue((const char*) value, len);
				if (isSet(ERROR_Code)) {
					theQueryErrorResponse.setErrorCode(lStrValue);
				}
				else if (isSet(ERROR_Message)) {
					theQueryErrorResponse.setErrorMessage(lStrValue);
				}
				else if (isSet(RequestId)) {
					theQueryErrorResponse.setRequestId(lStrValue);
				}
			}
		}

		template<class T>
		void SDBHandler<T>::endElement(const xmlChar * localname) {
			if (theIsSuccessful) {
				if (xmlStrEqual(localname, (xmlChar*) "BoxUsage")) {
					unsetState(BoxUsage);
				}
				else {
					responseEndElement(localname);
				}
			}
			else {
				if (xmlStrEqual(localname, (xmlChar*) "Code")) {
					unsetState(ERROR_Code);
				}
				else if (xmlStrEqual(localname, (xmlChar*) "Message")) {
					unsetState(ERROR_Message);
				}
				else if (xmlStrEqual(localname, (xmlChar*) "RequestID")) {
					unsetState(RequestId);
				}
			}
		}

		void CreateDomainHandler::responseStartElement(const xmlChar * localname,
				int nb_attributes, const xmlChar **attributes) {
			if (xmlStrEqual(localname, (xmlChar*) "CreateDomainResponse")) {
				theResponse = new CreateDomainResponse();
			}
		}

		void CreateDomainHandler::responseCharacters(const xmlChar * value, int len) {
		}

		void CreateDomainHandler::responseEndElement(const xmlChar * localname) {
		}

		void DeleteDomainHandler::responseStartElement(const xmlChar * localname,
				int nb_attributes, const xmlChar **attributes) {
			if (xmlStrEqual(localname, (xmlChar*) "DeleteDomainResponse")) {
				theResponse = new DeleteDomainResponse();
			}
		}

		void DeleteDomainHandler::responseCharacters(const xmlChar * value, int len) {
		}

		void DeleteDomainHandler::responseEndElement(const xmlChar * localname) {
		}

		void DomainMetadataHandler::responseStartElement(const xmlChar * localname,
				int nb_attributes, const xmlChar **attributes) {
			if (xmlStrEqual(localname, (xmlChar*) "DomainMetadataResponse")) {
				theResponse = new DomainMetadataResponse();
			}
			else if (xmlStrEqual(localname, (xmlChar*) "ItemCount")) {
				setState(ItemCount);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "ItemNamesSizeBytes")) {
				setState(ItemNamesSizeBytes);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeNameCount")) {
				setState(AttributeNameCount);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeNamesSizeBytes")) {
				setState(AttributeNamesSizeBytes);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeValueCount")) {
				setState(AttributeValueCount);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeValuesSizeBytes")) {
				setState(AttributeValuesSizeBytes);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "Timestamp")) {
				setState(Timestamp);
			}
		}

		void DomainMetadataHandler::responseCharacters(const xmlChar * value, int len) {
			std::string lValue((const char*) value, len);
			if (isSet(ItemCount)) {
        long l = atol(lValue.c_str());
        theResponse->setItemCount(l);
			}
			else if (isSet(ItemNamesSizeBytes)) {
        long l = atol(lValue.c_str());
        theResponse->setItemNamesSizeBytes(l);
			}
			else if (isSet(AttributeNameCount)) {
        int  i = atoi(lValue.c_str());
        theResponse->setAttributeNameCount(i);
			}
			else if (isSet(AttributeNamesSizeBytes)) {
        long l = atol(lValue.c_str());
        theResponse->setAttributeNamesSizeBytes(l);
			}
			else if (isSet(AttributeValueCount)) {
        long l = atol(lValue.c_str());
        theResponse->setAttributeValueCount(l);
			}
			else if (isSet(AttributeValuesSizeBytes)) {
        long l = atol(lValue.c_str());
        theResponse->setAttributeValuesSizeBytes(l);
			}
			else if (isSet(Timestamp)) {
        long l = atol(lValue.c_str());
        theResponse->setTimestamp(l);
			}
		}

		void DomainMetadataHandler::responseEndElement(const xmlChar * localname) {
			if (xmlStrEqual(localname, (xmlChar*) "ItemCount")) {
				unsetState(ItemCount);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "ItemNamesSizeBytes")) {
				unsetState(ItemNamesSizeBytes);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeNameCount")) {
				unsetState(AttributeNameCount);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeNamesSizeBytes")) {
				unsetState(AttributeNamesSizeBytes);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeValueCount")) {
				unsetState(AttributeValueCount);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeValuesSizeBytes")) {
				unsetState(AttributeValuesSizeBytes);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "Timestamp")) {
				unsetState(Timestamp);
			}
		}

		void ListDomainsHandler::responseStartElement(const xmlChar * localname,
				int nb_attributes, const xmlChar **attributes) {
			if (xmlStrEqual(localname, (xmlChar*) "ListDomainsResponse")) {
				theResponse = new ListDomainsResponse();
			}
			else if (xmlStrEqual(localname, (xmlChar*) "DomainName")) {
				setState(DomainName);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "NextToken")) {
				setState(NextToken);
			}
		}

		void ListDomainsHandler::responseCharacters(const xmlChar * value, int len) {
			std::string lValue((const char*) value, len);
			if (isSet(DomainName)) {
				theResponse->theDomainNames.push_back(lValue);
			}
			else if (isSet(NextToken)) {
				theResponse->theNextToken = lValue;
			}
		}

		void ListDomainsHandler::responseEndElement(const xmlChar * localname) {
			if (xmlStrEqual(localname, (xmlChar*) "DomainName")) {
				unsetState(DomainName);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "NextToken")) {
				unsetState(NextToken);
			}
		}

		void PutAttributesHandler::responseStartElement(const xmlChar * localname,
				int nb_attributes, const xmlChar **attributes) {
			if (xmlStrEqual(localname, (xmlChar*) "PutAttributesResponse")) {
				theResponse = new PutAttributesResponse();
			}
		}

		void PutAttributesHandler::responseCharacters(const xmlChar * value,
				int len) {
		}

		void PutAttributesHandler::responseEndElement(const xmlChar * localname) {
		}

		void BatchPutAttributesHandler::responseStartElement(const xmlChar * localname,
				int nb_attributes, const xmlChar **attributes) {
			if (xmlStrEqual(localname, (xmlChar*) "BatchPutAttributesResponse")) {
				theResponse = new BatchPutAttributesResponse();
			}
		}

		void BatchPutAttributesHandler::responseCharacters(const xmlChar * value,
				int len) {
		}

		void BatchPutAttributesHandler::responseEndElement(const xmlChar * localname) {
		}

		void DeleteAttributesHandler::responseStartElement(
				const xmlChar * localname, int nb_attributes,
				const xmlChar **attributes) {
			if (xmlStrEqual(localname, (xmlChar*) "DeleteAttributesResponse")) {
				theResponse = new DeleteAttributesResponse();
			}
		}

		void DeleteAttributesHandler::responseCharacters(const xmlChar * value,
				int len) {
		}

		void DeleteAttributesHandler::responseEndElement(const xmlChar * localname) {
		}

		void GetAttributesHandler::responseStartElement(const xmlChar * localname,
				int nb_attributes, const xmlChar **attributes) {
			if (xmlStrEqual(localname, (xmlChar*) "GetAttributesResponse")) {
				theResponse = new GetAttributesResponse();
			}
			else if (xmlStrEqual(localname, (xmlChar*) "Name")) {
				setState(Name);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "Value")) {
				setState(Value);
			}
		}

		void GetAttributesHandler::responseCharacters(const xmlChar * value,
				int len) {
			std::string lValue((const char*) value, len);
			if (isSet(Name)) {
				theResponse->theTmpName = lValue;
			}
			else if (isSet(Value)) {
				theResponse->theAttributes.push_back(AttributePair(
						theResponse->theTmpName, lValue));
			}
		}

		void GetAttributesHandler::responseEndElement(const xmlChar * localname) {
			if (xmlStrEqual(localname, (xmlChar*) "Name")) {
				unsetState(Name);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "Value")) {
				unsetState(Value);
			}
		}

		void QueryHandler::responseStartElement(const xmlChar * localname,
				int nb_attributes, const xmlChar **attributes) {
			if (xmlStrEqual(localname, (xmlChar*) "QueryResponse")) {
				theResponse = new SDBQueryResponse();
			}
			else if (xmlStrEqual(localname, (xmlChar*) "ItemName")) {
				setState(ItemName);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "NextToken")) {
				setState(NextToken);
			}
		}

		void QueryHandler::responseCharacters(const xmlChar * value, int len) {
			std::string lValue((const char*) value, len);
			if (isSet(ItemName)) {
				theResponse->theItemNames.push_back(lValue);
			}
			else if (isSet(NextToken)) {
				theResponse->theNextToken = lValue;
			}
		}

		void QueryHandler::responseEndElement(const xmlChar * localname) {
			if (xmlStrEqual(localname, (xmlChar*) "ItemName")) {
				unsetState(ItemName);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "NextToken")) {
				unsetState(NextToken);
			}
		}

		void
    QueryWithAttributesHandler::responseStartElement(const xmlChar * localname,
	                                     int nb_attributes, const xmlChar **attributes)
    {
			if (xmlStrEqual(localname, (xmlChar*) "QueryWithAttributesResponse")) {
				theResponse = new SDBQueryWithAttributesResponse();
			}
			else if (xmlStrEqual(localname, (xmlChar*) "Item")) {
        theResponse->theResponseElements.push_back(SDBQueryWithAttributesResponse::ResponseElement());
				setState(Item);
			}
			else if (isSet(Item) && xmlStrEqual(localname, (xmlChar*) "Name")) {
				setState(ItemName);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "Attribute")) {
        AttributePair lPair;
				theResponse->theResponseElements.back().Attributes.push_back(lPair);
				setState(Attribute);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeName")) {
				setState(AttributeName);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeValue")) {
				setState(AttributeValue);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "NextToken")) {
				setState(NextToken);
			}
		}

		void
    QueryWithAttributesHandler::responseCharacters(const xmlChar * value, int len)
    {
			std::string lValue((const char*) value, len);
			if (isSet(ItemName)) {
				theResponse->theResponseElements.back().ItemName = lValue;
			}
			if (isSet(AttributeName)) {
				theResponse->theResponseElements.back().Attributes.back().first = lValue;
			}
			if (isSet(AttributeValue)) {
				theResponse->theResponseElements.back().Attributes.back().second = lValue;
			}
			else if (isSet(NextToken)) {
				theResponse->theNextToken = lValue;
			}
		}

		void
    QueryWithAttributesHandler::responseEndElement(const xmlChar * localname)
    {
			if (xmlStrEqual(localname, (xmlChar*) "Item")) {
				unsetState(Item);
			}
			else if (isSet(Item) && xmlStrEqual(localname, (xmlChar*) "Name")) {
				unsetState(ItemName);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "Attribute")) {
				unsetState(Attribute);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeName")) {
				unsetState(AttributeName);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "AttributeValue")) {
				unsetState(AttributeValue);
			}
			else if (xmlStrEqual(localname, (xmlChar*) "NextToken")) {
				unsetState(NextToken);
			}
		}

	} /* namespace sqs  */
} /* namespace aws */
