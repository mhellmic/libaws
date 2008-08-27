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
#include <libaws/sqsexception.h>
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
      protected:
        friend class DeleteQueueHandler;
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
        friend class ListQueuesHandler;
        std::vector<std::string> theQueues;
        std::vector<std::string>::iterator theIterator;
    };

    class SendMessageResponse : public QueryResponse
    {
      public:
        const std::string&
        getMessageId() const;

        const std::string&
        getMD5OfMessageBody() const;

      protected:
        friend class SendMessageHandler;;
        std::string theMessageId;
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
          uint64_t    meta_data;
          std::string receipt_handle;
        };

        ~ReceiveMessageResponse();

        void
        open();

        bool
        next(Message& aMessage);

        void
        close();

        int
        getNumberOfRetrievedMessages() const;

      protected:
        friend class ReceiveMessageHandler;
        std::vector<Message> theMessages;
        std::vector<Message>::iterator theIterator;
    };

    class DeleteMessageResponse : public QueryResponse
    {
      protected:
        friend class DeleteMessageHandler;
    };

  } /* namespace sqs */
} /* namespace aws */

#endif
