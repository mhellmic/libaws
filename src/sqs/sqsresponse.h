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
#ifndef AWS_SQS_SQSRESPONSE_H
#define AWS_SQS_SQSRESPONSE_H

#include <vector>
#include <map>
#include <string>
#include <libaws/common.h>
#include <awsqueryresponse.h>

namespace aws {

  namespace sqs {

    class CreateQueueResponse : public QueryResponse
    {
      public:
        const std::string&
        getQueueUrl() const;

      protected:
        friend class CreateQueueHandler;
        std::string theQueueURL;
    };

    class DeleteQueueResponse : public QueryResponse
    {
      public:
        const std::string&
        getQueueUrl() const;

      protected:
        friend class DeleteQueueHandler;
        std::string theQueueURL;
    };

    class ListQueuesResponse : public QueryResponse
    {    
      public:
        void
        open();

        bool
        next(std::string& aQueueUrl);

        void
        close();

      protected:
        friend class ListQueueHandler;
        std::vector<std::string> theQueues;
        std::vector<std::string>::iterator theIterator;
    };

    class SendMessageResponse : public QueryResponse
    {
      public:
        const std::string&
        getMessageId() const;

        const std::string&
        getQueueName() const;

        const std::string&
        getMD5OfMessageBody() const;

      protected:
        friend class SendMessageHandler;;
        std::string theMessageId;
        std::string theQueueName;
        std::string theMD5OfMessageBody;
    };

    class ReceiveMessageResponse : public QueryResponse
    {
      public:
        struct Message 
        {
          const char* message_body;
          size_t      message_size;
          std::string message_md5;
          std::string message_id;
          std::string receipt_handle;
        };

        void
        open();

        bool
        next(Message& aMessage);

        void
        close();

        const std::string&
        getQueueName() const;

        int
        getNumberOfMessages() const;

        int
        getVisibilityTimeout() const;

        int
        getNumberOfRetrievedMessages() const;

      protected:
        friend class ReceiveMessageHandler;
        std::vector<Message> theMessages;
        std::vector<Message>::iterator theIterator;
        std::string theQueueName;
        int theNumberOfMessages;
        int theVisibilityTimeout;
    };

    class DeleteMessageResponse : public QueryResponse
    {
      public:
        const std::string&
        getReceiptHandle() const;

      protected:
        friend class DeleteMessageHandler;
        std::string theReceiptHandle;
    };

  } /* namespace sqs */
} /* namespace aws */

#endif
