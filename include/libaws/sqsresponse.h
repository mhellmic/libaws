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
#ifndef AWS_SQSRESPONSE_API_H
#define AWS_SQSRESPONSE_API_H

#include <vector>
#include <map>
#include <string>
#include <libaws/common.h>

namespace aws {

  // forward declaration of internal response classes
  namespace sqs {
      class CreateQueueResponse;
      class DeleteQueueResponse;
      class ListQueuesResponse;
      class SendMessageResponse;
      class ReceiveMessageResponse;
      class DeleteMessageResponse;
  } /* namespace sqs */

  template <class T>
  class SQSResponse : public SmartObject
  {
    public:
    	virtual ~SQSResponse();

      virtual const std::string&
      getRequestId() const;
      
      virtual double getKBOutTransfer() const;
      
      virtual double getKBInTransfer() const;

      virtual T*
      get() const { return theSQSResponse; }

    protected:
      T* theSQSResponse;
      SQSResponse(T*);
  };

  class CreateQueueResponse : public SQSResponse<sqs::CreateQueueResponse>
  {
    public:
      virtual ~CreateQueueResponse(){};

      const std::string&
      getQueueUrl() const;

    protected:
      friend class SQSConnectionImpl;
      CreateQueueResponse(sqs::CreateQueueResponse*);
  };

  class DeleteQueueResponse : public SQSResponse<sqs::DeleteQueueResponse>
  {
    public:
      virtual ~DeleteQueueResponse() {}

    protected:
      friend class SQSConnectionImpl;
      DeleteQueueResponse(sqs::DeleteQueueResponse*);
  };

  class ListQueuesResponse : public SQSResponse<sqs::ListQueuesResponse>
  {
    public:
      virtual ~ListQueuesResponse() {}

      void
      open();

      bool
      next(std::string& aQueueUrl);

      void
      close();

    protected:
      friend class SQSConnectionImpl;
      ListQueuesResponse(sqs::ListQueuesResponse*);
  };

  class SendMessageResponse : public SQSResponse<sqs::SendMessageResponse>
  {
    public:
      virtual ~SendMessageResponse() {}

      const std::string&
      getMessageId() const;

      const std::string&
      getMD5OfMessageBody() const;
      
      SendMessageResponse(sqs::SendMessageResponse*);
  };

  class ReceiveMessageResponse : public SQSResponse<sqs::ReceiveMessageResponse>
  {
    public:
      ~ReceiveMessageResponse() {}

      struct Message
      {
        const char* message_body;
        size_t      message_size;
        std::string message_md5;
        std::string message_id;
        uint64_t    meta_data;
        std::string receipt_handle;
      };

      void
      open();

      bool
      next(Message& aMessage);

      void
      close();

      int
      getNumberOfRetrievedMessages() const;

      ReceiveMessageResponse(sqs::ReceiveMessageResponse*);
  };

  class DeleteMessageResponse : public SQSResponse<sqs::DeleteMessageResponse>
  {
    public:
      ~DeleteMessageResponse() {}

    protected:
      friend class SQSConnectionImpl;
      DeleteMessageResponse(sqs::DeleteMessageResponse*);
  };

} /* namespace aws */
#endif
