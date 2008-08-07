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

      virtual CreateQueueResponsePtr
      createQueue(const std::string &aQueueName, int aDefaultVisibilityTimeout = -1) = 0;

      virtual DeleteQueueResponsePtr
      deleteQueue(const std::string &aQueueUrl) = 0;

      virtual ListQueuesResponsePtr
      listQueues(const std::string &aQueueNamePrefix = "") = 0;

      virtual SendMessageResponsePtr
      sendMessage(const std::string &aQueueUrl, const std::string &aMessageBody) = 0;

      virtual ReceiveMessageResponsePtr
      receiveMessage(const std::string &aQueueUrl,
                    int aNumberOfMessages = 0,
                    int aVisibilityTimeout = -1) = 0;

      virtual DeleteMessageResponsePtr
      deleteMessage(const std::string &aQueueUrl, const std::string &aReceiptHandle) = 0;

  }; /* class SQSConnection */

} /* namespace aws */
#endif
