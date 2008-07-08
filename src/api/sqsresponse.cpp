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

#include "sqs/sqsresponse.h"

namespace aws {
  
  template <class T>
  SQSResponse<T>::SQSResponse(T* aSQSResponse)
    : theSQSResponse(aSQSResponse) {}

  template <class T>
  SQSResponse<T>::~SQSResponse()
  {
    delete theSQSResponse;
  }

  template <class T>
  const std::string&
  SQSResponse<T>::getRequestId() const 
  { 
    return theSQSResponse->getRequestId();
  }
  
  /**
   * CreateQueueResponse
   */
  CreateQueueResponse::CreateQueueResponse(sqs::CreateQueueResponse* r)
    : SQSResponse<sqs::CreateQueueResponse>(r) {}

  const std::string&
  CreateQueueResponse::getQueueUrl() const
  {
    return theSQSResponse->getQueueUrl();
  }

  /**
   * DeleteQueueResponse
   */
  DeleteQueueResponse::DeleteQueueResponse(sqs::DeleteQueueResponse* r)
    : SQSResponse<sqs::DeleteQueueResponse>(r) {}

  const std::string&
  DeleteQueueResponse::getQueueUrl() const
  {
    return theSQSResponse->getQueueUrl();
  }

  /**
   * ListQueuesResponse
   */
  ListQueuesResponse::ListQueuesResponse(sqs::ListQueuesResponse* r)
    : SQSResponse<sqs::ListQueuesResponse>(r) {}

  const std::string&
  ListQueuesResponse::getQueuePrefix() const
  {
    return theSQSResponse->getQueuePrefix();
  }

  /**
   * SendMessageResponse
   */
  SendMessageResponse::SendMessageResponse(sqs::SendMessageResponse* r)
    : SQSResponse<sqs::SendMessageResponse>(r) {}

  const std::string&
  SendMessageResponse::getMessageId() const
  {
    return theSQSResponse->getMessageId();
  }

  const std::string&
  SendMessageResponse::getQueueName() const
  {
    return theSQSResponse->getQueueName();
  }

  /**
   * PeekMessageResponse
   */
  PeekMessageResponse::PeekMessageResponse(sqs::PeekMessageResponse* r)
    : SQSResponse<sqs::PeekMessageResponse>(r) {}

  const std::string&
  PeekMessageResponse::getMessageId() const
  {
    return theSQSResponse->getMessageId();
  }

  const std::string&
  PeekMessageResponse::getQueueName() const
  {
    return theSQSResponse->getQueueName();
  }

  const char*
  PeekMessageResponse::getContent()
  {
    return theSQSResponse->getContent();
  }

  int
  PeekMessageResponse::getSize()
  {
    return theSQSResponse->getSize();
  }

  /**
   * ReceiveMessageResponse
   */
  ReceiveMessageResponse::ReceiveMessageResponse(sqs::ReceiveMessageResponse* r)
    : SQSResponse<sqs::ReceiveMessageResponse>(r) {}

  const std::string&
  ReceiveMessageResponse::getQueueName() const
  {
    return theSQSResponse->getQueueName();
  }

  int
  ReceiveMessageResponse::getNumberOfMessages()
  {
    return theSQSResponse->getNumberOfMessages();
  }

  int
  ReceiveMessageResponse::getVisibilityTimeout()
  {
    return theSQSResponse->getVisibilityTimeout();
  }

  int
  ReceiveMessageResponse::getNumberOfRetrievedMessages()
  {
    return theSQSResponse->getNumberOfRetrievedMessages();
  }

  bool
  ReceiveMessageResponse::isEmpty()
  {
    return theSQSResponse->isEmpty();
  }

  /**
   * DeleteMessageResponse
   */
  DeleteMessageResponse::DeleteMessageResponse(sqs::DeleteMessageResponse* r)
    : SQSResponse<sqs::DeleteMessageResponse>(r) {}

  const std::string&
  DeleteMessageResponse::getQueueName() const
  {
    return theSQSResponse->getQueueName();
  }

  const std::string&
  DeleteMessageResponse::getMessageId() const
  {
    return theSQSResponse->getMessageId();
  }
} /* namespace aws */

