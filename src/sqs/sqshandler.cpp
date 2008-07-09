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
    CreateQueueHandler::startElement ( const std::string& localname, int nb_attributes, const xmlChar ** attributes )
    {
      if (localname.compare("RequestId") == 0) {
        setState(RequestId);
      } else if (localname.compare("QueueUrl") == 0) {
        setState(QueueUrl);
      }
    }
        
    void
    CreateQueueHandler::characters ( const std::string& value )
    {
      if (isSet(RequestId)) {
        theCreateQueueResponse->theRequestId = value;
      } else if (isSet(QueueUrl)) {
        theCreateQueueResponse->theQueueURL = value;
      }
    }
        
    void
    CreateQueueHandler::endElement ( const std::string& localname )
    {
      if (localname.compare("RequestId") == 0) {
        unsetState(RequestId);
      } else if (localname.compare("QueueUrl") == 0) {
        unsetState(QueueUrl);
      }
    }

  } /* namespace sqs  */
} /* namespace aws */
