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
#include "sqs/sqshandler.h"
#include "sqs/sqsresponse.h"
#include <iostream>

#include <string>

using namespace aws;

namespace aws {
  namespace sqs {

    void 
    QueueErrorHandler::startElement ( const xmlChar *  localname, 
                                      int nb_attributes, 
                                      const xmlChar ** attributes ) {
      if (xmlStrEqual ( localname, BAD_CAST "ErrorResponse" ) ) {
        theIsSuccessful = false;
        theQueryErrorResponse = new QueryErrorResponse();
      }else if (theIsSuccessful ) {
        responseStartElement ( localname, nb_attributes, attributes );
      }else if(xmlStrEqual ( localname, BAD_CAST "Code" )) {
        setState ( ERROR_Code );
      }else if(xmlStrEqual ( localname, BAD_CAST "Message" )) {
        setState ( ERROR_Message );
      }else if(xmlStrEqual ( localname, BAD_CAST "RequestID" )) {
        setState ( RequestId );
      }

    }
    
    void QueueErrorHandler::characters ( const xmlChar *  value,         
                                         int len ) {
                                           
      if (theIsSuccessful ) {
        responseCharacters ( value, len );
      } else {
        std::string lStrValue(( const char* ) value, len);
        
        if(isSet ( ERROR_Code)) {
          theQueryErrorResponse->setErrorCode(lStrValue);
        }else if(isSet ( ERROR_Message)) {
          theQueryErrorResponse->setErrorMessage(lStrValue);
        }else if(isSet ( RequestId)) {
          theQueryErrorResponse->setRequestId(lStrValue);
        }  
      }
    }
    
    void 
    QueueErrorHandler::endElement ( const xmlChar *  localname ) {
      responseEndElement ( localname );
    }

    void
    CreateQueueHandler::responseStartElement ( const xmlChar * localname, int nb_attributes, const xmlChar ** attributes )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "CreateQueueResponse" ) ) {
        theCreateQueueResponse = new CreateQueueResponse();
      } else if ( xmlStrEqual ( localname, BAD_CAST "RequestId" ) ) {
        setState ( RequestId );
      } else if ( xmlStrEqual ( localname, BAD_CAST "QueueUrl" ) ) {
        setState ( QueueUrl );
      }
    }

    void
    CreateQueueHandler::responseCharacters ( const xmlChar *  value, int len )
    {
      //std::string test((const char*)value, len);
      //std::cout << "value:" << test << std::endl;
      if ( isSet ( RequestId ) ) {
        theCreateQueueResponse->theRequestId.append ( ( const char* ) value, len );
      } else if ( isSet ( QueueUrl ) ) {
        theCreateQueueResponse->theQueueURL.append ( ( const char* ) value, len );
      }
    }

    void
    CreateQueueHandler::responseEndElement ( const xmlChar * localname )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "RequestId" ) ) {
        unsetState ( RequestId );
      } else if ( xmlStrEqual ( localname, BAD_CAST "QueueUrl" ) ) {
        unsetState ( QueueUrl );
      }
    }

  } /* namespace sqs  */
} /* namespace aws */
