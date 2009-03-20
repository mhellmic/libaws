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

#include "sdb/sdbconnection.h"
#include "sdb/sdbresponse.h"
#include "sdb/sdbhandler.h"

#include <libaws/sdbexception.h>

#include <sstream>
#include <memory>
#include <cassert>


using namespace aws;

namespace aws {
  namespace sdb {

    const std::string SDBConnection::DEFAULT_VERSION = "2007-11-07";
    const std::string SDBConnection::DEFAULT_HOST = "sdb.amazonaws.com";

    SDBConnection::SDBConnection ( const std::string& aAccessKeyId,
                                   const std::string& aSecretAccessKey,
                                   const std::string& aCustomHost )
        : AWSQueryConnection ( aAccessKeyId, aSecretAccessKey, aCustomHost.size()==0?DEFAULT_HOST:aCustomHost,
                               DEFAULT_VERSION )
    {
    }

    CreateDomainResponse*
    SDBConnection::createDomain ( const std::string& aDomainName ) {

      ParameterMap lMap;
      lMap.insert ( ParameterPair ( "DomainName", aDomainName ) );

      CreateDomainHandler lHandler;
      makeQueryRequest ( "CreateDomain", &lMap, &lHandler );
      if ( lHandler.isSuccessful() ) {
        CreateDomainResponse* lPtr = lHandler.theResponse;
        setCommons(lHandler, lPtr);
        return lPtr;
      }
			else {
				throw CreateDomainException(lHandler.getQueryErrorResponse());
      }
    }

    DeleteDomainResponse*
    SDBConnection::deleteDomain ( const std::string& aDomainName ) {

      ParameterMap lMap;
      lMap.insert ( ParameterPair ( "DomainName", aDomainName ) );

      DeleteDomainHandler lHandler;
      makeQueryRequest ( "DeleteDomain", &lMap, &lHandler );
      if ( lHandler.isSuccessful() ) {
      	DeleteDomainResponse* lPtr = lHandler.theResponse;
        setCommons(lHandler, lPtr);
        return lPtr;
      }
			else {
				throw DeleteDomainException(lHandler.getQueryErrorResponse());
      }
    }

    ListDomainsResponse*
    SDBConnection::listDomains(int aMaxNumberOfDomains, const std::string& aNextToken) {

      ParameterMap lMap;
      if (aMaxNumberOfDomains > 0 ) {
        std::stringstream s;
        s << aMaxNumberOfDomains;
        lMap.insert ( ParameterPair ( "MaxNumberOfDomains", s.str() ) );
      }
      if (aNextToken != std::string("")) {
      	lMap.insert( ParameterPair ( "NextToken", aNextToken ) );
      }

      ListDomainsHandler lHandler;
      makeQueryRequest ( "ListDomains", &lMap, &lHandler );
      if ( lHandler.isSuccessful() ) {
      	ListDomainsResponse* lPtr = lHandler.theResponse;
        setCommons(lHandler, lPtr);
        return lPtr;
      }
			else {
				throw ListDomainsException(lHandler.getQueryErrorResponse());
      }
    }

    PutAttributesResponse*
    SDBConnection::putAttributes ( const std::string& aDomainName, const std::string& aItemName,
			const std::vector<Attribute>& attributes ) {

      ParameterMap lMap;
      lMap.insert ( ParameterPair ( "DomainName", aDomainName ) );
      lMap.insert ( ParameterPair ( "ItemName", aItemName ) );
      insertAttParameter(lMap, attributes, true);

      PutAttributesHandler lHandler;
      makeQueryRequest ( "PutAttributes", &lMap, &lHandler );
      if ( lHandler.isSuccessful() ) {
      	PutAttributesResponse* lPtr = lHandler.theResponse;
        setCommons(lHandler, lPtr);
        return lPtr;
      }
			else {
				throw PutAttributesException(lHandler.getQueryErrorResponse());
      }
    }

    DeleteAttributesResponse*
    SDBConnection::deleteAttributes ( const std::string& aDomainName, const std::string& aItemName,
			const std::vector<Attribute>& attributes ) {

      ParameterMap lMap;
      lMap.insert ( ParameterPair ( "DomainName", aDomainName ) );
      lMap.insert ( ParameterPair ( "ItemName", aItemName ) );
      insertAttParameter(lMap, attributes, false);

      DeleteAttributesHandler lHandler;
      makeQueryRequest ( "DeleteAttributes", &lMap, &lHandler );
      if ( lHandler.isSuccessful() ) {
      	DeleteAttributesResponse* lPtr = lHandler.theResponse;
        setCommons(lHandler, lPtr);
        return lPtr;
      }
			else {
				throw DeleteAttributesException(lHandler.getQueryErrorResponse());
      }
    }

    GetAttributesResponse*
    SDBConnection::getAttributes ( const std::string& aDomainName, const std::string& aItemName,
			const std::string& attributeName ) {

      ParameterMap lMap;
      lMap.insert ( ParameterPair ( "DomainName", aDomainName ) );
      lMap.insert ( ParameterPair ( "ItemName", aItemName ) );
      if (attributeName != std::string("")) {
        lMap.insert ( ParameterPair ( "AttributeName", attributeName ) );
      }

      GetAttributesHandler lHandler;
      makeQueryRequest ( "GetAttributes", &lMap, &lHandler );
      if ( lHandler.isSuccessful() ) {
      	GetAttributesResponse* lPtr = lHandler.theResponse;
        setCommons(lHandler, lPtr);
        return lPtr;
      }
			else {
				throw GetAttributesException(lHandler.getQueryErrorResponse());
      }
    }

    SDBQueryResponse*
    SDBConnection::query ( const std::string& aDomainName, const std::string& aQueryExpression,
    		int aMaxNumberOfItems, const std::string& aNextToken ) {

      ParameterMap lMap;
      lMap.insert ( ParameterPair ( "DomainName", aDomainName ) );
      lMap.insert ( ParameterPair ( "QueryExpression", aQueryExpression ) );
      if (aMaxNumberOfItems > 0 ) {
        std::stringstream s;
        s << aMaxNumberOfItems;
        lMap.insert ( ParameterPair ( "MaxNumberOfItems", s.str() ) );
      }
      if (aNextToken != std::string("") ) {
        lMap.insert ( ParameterPair ( "NextToken", aNextToken ) );
      }

      QueryHandler lHandler;
      makeQueryRequest ( "Query", &lMap, &lHandler );
      if ( lHandler.isSuccessful() ) {
      	SDBQueryResponse* lPtr = lHandler.theResponse;
        setCommons(lHandler, lPtr);
        return lPtr;
      }
			else {
				throw QueryException(lHandler.getQueryErrorResponse());
      }
    }

    void
    SDBConnection::insertAttParameter(ParameterMap& aMap, const std::vector<Attribute>& attributes, bool insertReplaces) {
      int lAttNr = 0;
      for(std::vector<Attribute>::const_iterator iter = attributes.begin(); iter != attributes.end(); iter++) {
      	std::stringstream a;
      	a << "Attribute." << lAttNr << ".Name";
      	aMap.insert(ParameterPair(a.str(), iter->getName()));
      	std::stringstream b;
      	b << "Attribute." << lAttNr << ".Value";
      	aMap.insert(ParameterPair(b.str(), iter->getValue()));
      	if (insertReplaces && iter->isReplace()) {
					std::stringstream c;
					c << "Attribute." << lAttNr << ".Replace";
					aMap.insert(ParameterPair(c.str(), std::string("true")));
      	}
      	lAttNr++;
      }
    }

  }
}//namespaces

