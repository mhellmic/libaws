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

#include <string>

using namespace aws;

namespace aws {
  namespace sdb {

    template <class T>
    void
    SDBHandler<T>::startElement ( const xmlChar *  localname,
                                  int nb_attributes,
                                  const xmlChar ** attributes ) {
      if ( xmlStrEqual ( localname, BAD_CAST "ErrorResponse" ) ) {
        theIsSuccessful = false;
        theQueryErrorResponse = QueryErrorResponse();
      }else if ( theIsSuccessful && xmlStrEqual ( localname, BAD_CAST "BoxUsage" ) ) {
        setState ( BoxUsage );
      }else if ( theIsSuccessful ) {
        responseStartElement ( localname, nb_attributes, attributes );
      }else if ( xmlStrEqual ( localname, BAD_CAST "Code" ) ) {
        setState ( ERROR_Code );
      }else if ( xmlStrEqual ( localname, BAD_CAST "Message" ) ) {
        setState ( ERROR_Message );
      }else if ( xmlStrEqual ( localname, BAD_CAST "RequestID" ) ) {
        setState ( RequestId );
      }

    }

    template <class T>
    void
    SDBHandler<T>::characters ( const xmlChar *  value,
                                int len ) {

      if ( theIsSuccessful ) {
        if( isSet(BoxUsage)){
          std::string lStrValue ( ( const char* ) value, len );
          theResponse->setBoxUsage(lStrValue);
        }else{
          responseCharacters ( value, len );
        }
      } else {
        std::string lStrValue ( ( const char* ) value, len );
        if ( isSet ( ERROR_Code ) ) {
          theQueryErrorResponse.setErrorCode ( lStrValue );
        }else if ( isSet ( ERROR_Message ) ) {
          theQueryErrorResponse.setErrorMessage ( lStrValue );
        }else if ( isSet ( RequestId ) ) {
          theQueryErrorResponse.setRequestId ( lStrValue );
        }
      }
    }

    template <class T>
    void
    SDBHandler<T>::endElement ( const xmlChar *  localname ) {
      if ( theIsSuccessful ) {
        if ( xmlStrEqual ( localname, BAD_CAST "BoxUsage" ) ) {
          unsetState ( BoxUsage );
        }else{
          responseEndElement ( localname );
        }
      }else{
        if ( xmlStrEqual ( localname, BAD_CAST "Code" ) ) {
          unsetState ( ERROR_Code );
        } else if ( xmlStrEqual ( localname, BAD_CAST "Message" ) ) {
          unsetState ( ERROR_Message );
        } else if ( xmlStrEqual ( localname, BAD_CAST "RequestID" ) ) {
          unsetState ( RequestId );
        }
      }
    }

    void
        CreateDomainHandler::responseStartElement ( const xmlChar * localname, int nb_attributes, const xmlChar ** attributes )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "CreateDomainResponse" ) ) {
        theResponse = new CreateDomainResponse();
      }
    }

    void
        CreateDomainHandler::responseCharacters ( const xmlChar *  value, int len )
    {

    }

    void
        CreateDomainHandler::responseEndElement ( const xmlChar * localname )
    {

    }

  } /* namespace sqs  */
} /* namespace aws */
