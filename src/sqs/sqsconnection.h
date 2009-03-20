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
#ifndef AWS_SQS_SQSCONNECTION_H
#define AWS_SQS_SQSCONNECTION_H

#include "common.h"

#include <map>
#include <iostream>

#include "awsqueryconnection.h"

namespace aws {

  namespace sqs {

    class CreateQueueResponse;
    class DeleteQueueResponse;
    class ListQueuesResponse;
    class SendMessageResponse;
    class ReceiveMessageResponse;
    class DeleteMessageResponse;

    class SQSConnection : public AWSQueryConnection
    {
      public:
        static const std::string DEFAULT_VERSION;
        static const std::string DEFAULT_HOST;

      public:
        SQSConnection(const std::string& aAccessKeyId,
                      const std::string& aSecretAccessKey,
                      const std::string& aCustomHost);

        SQSConnection(const std::string& aAccessKeyId,
                      const std::string& aSecretAccessKey,
                      const std::string& aHost,
                      int aPort,
                      bool aIsSecure);

        virtual CreateQueueResponse*
        createQueue ( const std::string &aQueueName, int aDefaultVisibilityTimeout );

        virtual DeleteQueueResponse*
        deleteQueue ( const std::string &aQueueUrl );

        virtual ListQueuesResponse*
        listQueues ( const std::string &aQueueNamePrefix = "" );

        virtual SendMessageResponse*
        sendMessage ( const std::string &aQueueUrl, const std::string &aMessageBody );
        
        virtual SendMessageResponse* sendMessage (const std::string &aQueueUrl, ParameterMap& lMap);

        virtual ReceiveMessageResponse*
        receiveMessage( const std::string &aQueueUrl,
                        int aNumberOfMessages = 0,
                        int aVisibilityTimeout = -1);
        
        virtual ReceiveMessageResponse*
        receiveMessage (const std::string &aQueueUrl,
                        ParameterMap& lMap);

        virtual DeleteMessageResponse*
        deleteMessage( const std::string &aQueueUrl, const std::string &aReceiptHandle);
    };

  } /* namespace sqs  */
} /* namespace aws */

#endif        /* !AWS_SQS_SQSCONNECTION_H */
