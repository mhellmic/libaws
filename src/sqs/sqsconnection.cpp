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
#include <cassert>


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
  SQSConnection::createQueue ( const std::string &aQueueName, int aDefaultVisibilityTimeout)
  {
    ParameterMap lMap;
    lMap.insert ( ParameterPair ( "QueueName", aQueueName ) );
 
    if (aDefaultVisibilityTimeout > -1 ) {
      std::stringstream s;
      s << aDefaultVisibilityTimeout;
      lMap.insert ( ParameterPair ( "DefaultVisibilityTimeout", s.str() ) );
    }
 
    CreateQueueHandler lHandler;
    makeQueryRequest ( "CreateQueue", &lMap, &lHandler );
    if(lHandler.isSuccessful()){
      return lHandler.theCreateQueueResponse;
    }else{
      assert(false); //Matthias you need to throw the exception here! Get the ErrorResponse from lHandler.getQueryErrorResponse() 
    }
  }
  
  DeleteQueueResponse*
  SQSConnection::deleteQueue(const std::string &aQueueUrl, bool aForceDeletion)
  {
  }

  ListQueuesResponse*
  SQSConnection::listQueues(const std::string &aQueueNamePrefix)
  {
  }

  SendMessageResponse*
  SQSConnection::sendMessage(const std::string &aQueueName, const char* aContent, size_t aContentSize)
  {
  }

  ReceiveMessageResponse*
  SQSConnection::receiveMessage(const std::string &aQueueName,
                                int aNumberOfMessages,
                                int aVisibilityTimeout)
  {
  }

  DeleteMessageResponse*
  SQSConnection::deleteMessage(const std::string &aReceiptHandle)
  {
  }

}}//namespaces

