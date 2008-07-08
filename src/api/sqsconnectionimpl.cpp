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
#include <libaws/sqsresponse.h>

#include "sqs/sqsconnection.h"
#include "api/sqsconnectionimpl.h"

namespace aws {

  CreateQueueResponsePtr
  SQSConnectionImpl::createQueue(const std::string &aQueueName, int aDefaultVisibilityTimeout)
  {
    return new CreateQueueResponse(theConnection->createQueue(aQueueName, aDefaultVisibilityTimeout));
  }

  DeleteQueueResponsePtr
  SQSConnectionImpl::deleteQueue(const std::string &aQueueUrl, bool aForceDeletion)
  {
    return new DeleteQueueResponse(theConnection->deleteQueue(aQueueUrl, aForceDeletion));
  }

  ListQueuesResponsePtr
  SQSConnectionImpl::listQueues(const std::string &aQueueNamePrefix)
  {
    return new theConnection->listQueues(aQueueNamePrefix);
  }

  // Message handling functions    
  SendMessageResponsePtr
  SQSConnectionImpl::sendMessage(const std::string &aQueueName, const char* aContent, size_t aContentSize)
  {
    return new theConnection->sendMessage(aQueueName, aContent, aContentSize);
  }

  SendMessageResponsePtr
  SQSConnectionImpl::sendMessage(const std::string &aQueueName, const std::string &aContent)
  {
    return new theConnection->sendMessage(aQueueName, aContent);
  }

  PeekMessageResponsePtr
  SQSConnectionImpl::peekMessage(const std::string &aQueueName, const std::string &aMessageId)
  {
    return new theConnection->peekMessage(aQueueName, aMessageId);
  }

  ReceiveMessageResponsePtr
  SQSConnectionImpl::receiveMessage(const std::string &aQueueName,
                int aNumberOfMessages,
                int aVisibilityTimeout)
  {
    return new theConnection->receiveMessage(aQueueName, aNumberOfMessages, aVisibilityTimeout);
  }

  DeleteMessageResponsePtr
  SQSConnectionImpl::deleteMessage(const std::string &aQueueName, const std::string &aMessageId)
  {
    return new theConnection->deleteMessage(aQueueName, aMessageId);
  }

  SQSConnectionImpl::SQSConnectionImpl(const std::string& aAccessKeyId, 
                                       const std::string& aSecretAccessKey)
  {
    theConnection = new sqs::SQSConnection(aAccessKeyId, aSecretAccessKey);
  }

  SQSConnectionImpl::~SQSConnectionImpl() 
  {
    delete theConnection;
  }

} /* namespace aws */
