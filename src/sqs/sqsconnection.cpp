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

#include "sqs/sqsconnection.h"
#include "sqs/sqsresponse.h"
#include "sqs/sqshandler.h"

#include <sstream>
#include <memory>


using namespace aws;

namespace aws { namespace sqs {
  
  const std::string SQSConnection::DEFAULT_VERSION = "2008-01-01";
  const std::string SQSConnection::DEFAULT_HOST = "queue.amazonaws.com";
  
  SQSConnection::SQSConnection(const std::string& aAccessKeyId, 
                               const std::string& aSecretAccessKey) 
  : AWSQueryConnection(aAccessKeyId, aSecretAccessKey, DEFAULT_HOST, DEFAULT_VERSION)
  {
    
  }
  
  CreateQueueResponse*
  SQSConnection::createQueue ( const std::string &aQueueName, int aDefaultVisibilityTimeout ){

    ParameterMap lMap;
    lMap.insert ( ParameterPair ( "QueueName", aQueueName ) );
 
    if (aDefaultVisibilityTimeout > -1 ) {
      std::stringstream s;
      s << aDefaultVisibilityTimeout;
      lMap.insert ( ParameterPair ( "DefaultVisibilityTimeout", s.str() ) );
    }
 
   
    CreateQueueHandler lHandler;
    
    makeQueryRequest ( "CreateQueue", &lMap, &lHandler );

    return lHandler.theCreateQueueResponse;
  }
  
}}//namespaces

