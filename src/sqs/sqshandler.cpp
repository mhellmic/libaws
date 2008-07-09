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

#include <string>

using namespace aws;

namespace aws {
  namespace sqs {
    
    void
    CreateQueueHandler::startElement ( const xmlChar * localname, int nb_attributes, const xmlChar ** attributes )
    {
      if (xmlStrEqual(localname, BAD_CAST "CreateQueueResponse") ) {
        theCreateQueueResponse = new CreateQueueResponse();
      } else if (xmlStrEqual(localname, BAD_CAST "RequestId") ) {
        setState(RequestId);
      } else if (xmlStrEqual(localname, BAD_CAST "QueueUrl")) {
        setState(QueueUrl);
      }
    }
        
    void
    CreateQueueHandler::characters ( const xmlChar *  value, int len )
    {
      if (isSet(RequestId)) {
        theCreateQueueResponse->theRequestId.append((const char*)value, len);
      } else if (isSet(QueueUrl)) {
        theCreateQueueResponse->theQueueURL.append((const char*)value, len);
      }
    }
        
    void
    CreateQueueHandler::endElement ( const xmlChar * localname )
    {
      if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
        unsetState(RequestId);
      } else if (xmlStrEqual(localname, BAD_CAST "QueueUrl")) {
        unsetState(QueueUrl);
      }
    }

  } /* namespace sqs  */
} /* namespace aws */
