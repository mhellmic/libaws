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
#ifndef AWS_SQS_SQSCONNECTION_API_H
#define AWS_SQS_SQSCONNECTION_API_H

#include <istream>
#include <map>
#include <libaws/common.h>

namespace aws {

  class SQSConnection : public SmartObject
  {
    public:
      virtual ~SQSConnection() {}

      CreateQueueResponsePtr
      createQueue(const std::string &aQueueName, int aDefaultVisibilityTimeout = -1) = 0;

#if 0
      DeleteQueueResponsePtr
      deleteQueue(const std::string &aQueueUrl, bool aForceDeletion = false) = 0;

      ListQueuesResponsePtr
      listQueues(const std::string &aQueueNamePrefix = "") = 0;

      SendMessageResponsePtr
      sendMessage(const std::string &aQueueName, const char* aContent, size_t aContentSize) = 0;

      SendMessageResponsePtr
      sendMessage(const std::string &aQueueName, const std::string &aContent) = 0;

      PeekMessageResponsePtr
      peekMessage(const std::string &aQueueName, const std::string &aMessageId) = 0;

      ReceiveMessageResponsePtr
      receiveMessage(const std::string &aQueueName,
                    int aNumberOfMessages = -1,
                    int aVisibilityTimeout = -1) = 0;

      DeleteMessageResponsePtr
      deleteMessage(const std::string &aQueueName, const std::string &aMessageId) = 0;
                                                           
#endif

  }; /* class SQSConnection */

} /* namespace aws */
#endif
