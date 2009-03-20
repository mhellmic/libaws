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
  SQSConnectionImpl::deleteQueue(const std::string &aQueueUrl )
  {
    return new DeleteQueueResponse(theConnection->deleteQueue(aQueueUrl ));
  }

  ListQueuesResponsePtr
  SQSConnectionImpl::listQueues(const std::string &aQueueNamePrefix)
  {
    return new ListQueuesResponse(theConnection->listQueues(aQueueNamePrefix));
  }

  // Message handling functions
  SendMessageResponsePtr
  SQSConnectionImpl::sendMessage(const std::string &aQueueUrl, const std::string &aMessageBody )
  {
    return new SendMessageResponse(theConnection->sendMessage(aQueueUrl, aMessageBody));
  }

  ReceiveMessageResponsePtr
  SQSConnectionImpl::receiveMessage(const std::string &aQueueUrl,
                int aNumberOfMessages,
                int aVisibilityTimeout)
  {
    return new ReceiveMessageResponse(theConnection->receiveMessage(aQueueUrl,
                                                                    aNumberOfMessages,
                                                                    aVisibilityTimeout));
  }

  DeleteMessageResponsePtr
  SQSConnectionImpl::deleteMessage(const std::string &aQueueUrl,
								const std::string &aReceiptHandle)
  {
    return new DeleteMessageResponse(theConnection->deleteMessage(aQueueUrl, aReceiptHandle));
  }

  SQSConnectionImpl::SQSConnectionImpl(const std::string& aAccessKeyId,
                                       const std::string& aSecretAccessKey,
                                       const std::string& aCustomHost)
  {
    theConnection = new sqs::SQSConnection(aAccessKeyId, aSecretAccessKey, aCustomHost);
  }

  SQSConnectionImpl::SQSConnectionImpl(const std::string& aAccessKeyId,
                                       const std::string& aSecretAccessKey,
                                       const std::string& aCustomHost,
                                       int aPort,
                                       bool aIsSecure)
  {
    theConnection = new sqs::SQSConnection(aAccessKeyId, aSecretAccessKey, aCustomHost, aPort, aIsSecure);
  }

  SQSConnectionImpl::~SQSConnectionImpl()
  {
    delete theConnection;
  }

} /* namespace aws */
