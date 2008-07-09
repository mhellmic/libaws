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

    class SQSConnection : public AWSQueryConnection
    {
      public:
        static const std::string DEFAULT_VERSION;
        static const std::string DEFAULT_HOST;
        
      
      public:
        SQSConnection(const std::string& aAccessKeyId, 
                      const std::string& aSecretAccessKey);
      
        CreateQueueResponse*
        createQueue ( const std::string &aQueueName, int aDefaultVisibilityTimeout );

        /*
        DeleteQueueResponse*
        deleteQueue ( const std::string &aQueueUrl, bool aForceDeletion = false );

        ListQueuesResponse*
        listQueues ( const std::string &aQueueNamePrefix = "" );

        SendMessageResponse*
        sendMessage ( const std::string &aQueueName, const char* aContent, size_t aContentSize );

        SendMessageResponse*
        sendMessage ( const std::string &aQueueName, const std::string &aContent );

        PeekMessageResponse*
        peekMessage ( const std::string &aQueueName, const std::string &aMessageId );

        ReceiveMessageResponse*
        receiveMessage ( const std::string &aQueueName,
                         int aNumberOfMessages = -1,
                         int aVisibilityTimeout = -1 );

        DeleteMessageResponse*
        deleteMessage ( const std::string &aQueueName, const std::string &aMessageId );*/

    };

  } /* namespace sqs  */
} /* namespace aws */

#endif        /* !AWS_SQS_SQSCONNECTION_H */
