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
#ifndef AWS_SQS_SQSCONNECTIONIMPL_H
#define AWS_SQS_SQSCONNECTIONIMPL_H

#include "common.h"
#include <libaws/sqsconnection.h>

namespace aws {

  namespace sqs {
    class SQSConnection;
  }

  class SQSConnectionImpl : public SQSConnection
  {
    public:
      virtual ~SQSConnectionImpl();

      virtual CreateQueueResponsePtr
      createQueue(const std::string &aQueueName, int aDefaultVisibilityTimeout = -1);

      virtual DeleteQueueResponsePtr
      deleteQueue(const std::string &aQueueUrl);

      virtual ListQueuesResponsePtr
      listQueues(const std::string &aQueueNamePrefix = "");

      virtual SendMessageResponsePtr
      sendMessage(const std::string &aQueueUrl, const std::string &aMessageBody);

      virtual ReceiveMessageResponsePtr
      receiveMessage(const std::string &aQueueUrl,
                    int aNumberOfMessages = 0,
                    int aVisibilityTimeout = -1);

      virtual DeleteMessageResponsePtr
      deleteMessage(const std::string &aQueueUrl, const std::string &aReceiptHandle);

    protected:
      // only the factory can create us
      friend class AWSConnectionFactoryImpl;
      SQSConnectionImpl(const std::string& aAccessKeyId, const std::string& aSecretAccessKey,
          const std::string& aCustomHost);
      SQSConnectionImpl(const std::string& aAccessKeyId, const std::string& aSecretAccessKey,
      		const std::string& aHost, int aPort, bool aIsSecure);

      sqs::SQSConnection* theConnection;

  }; /* class S3ConnectionImpl */

} /* namespace aws */
#endif
