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

#include <sstream>
#include <memory>
#include <cassert>


using namespace aws;

namespace aws {
  namespace sdb {

    const std::string SDBConnection::DEFAULT_VERSION = "2007-11-07";
    const std::string SDBConnection::DEFAULT_HOST = "sdb.amazonaws.com";

    SDBConnection::SDBConnection ( const std::string& aAccessKeyId,
                                   const std::string& aSecretAccessKey )
        : AWSQueryConnection ( aAccessKeyId, aSecretAccessKey, DEFAULT_HOST, DEFAULT_VERSION )
    {

    }

    CreateDomainResponse*
    SDBConnection::createDomain ( const std::string &aDomainName ) {

      ParameterMap lMap;
      lMap.insert ( ParameterPair ( "DomainName", aDomainName ) ); 

      CreateDomainHandler lHandler;
      makeQueryRequest ( "CreateDomain", &lMap, &lHandler );
      if ( lHandler.isSuccessful() ) {
        CreateDomainResponse* lPtr = lHandler.theResponse;
        return lPtr;
      }else{
        assert ( false ); //Matthias you need to throw the exception here! Get the ErrorResponse from lHandler.getQueryErrorResponse()
      }
    }

  }
}//namespaces

