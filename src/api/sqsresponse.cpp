
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
  
  template <class T>
  double SQSResponse<T>::getKBOutTransfer() const{
    return theSQSResponse->getOutTransfer() / 1024;
  }
      
  template <class T>
  double SQSResponse<T>::getKBInTransfer() const{
    return  theSQSResponse->getInTransfer() / 1024;
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

  /**
   * ListQueuesResponse
   */
  ListQueuesResponse::ListQueuesResponse(sqs::ListQueuesResponse* r)
    : SQSResponse<sqs::ListQueuesResponse>(r) {}

  void
  ListQueuesResponse::open()
  {
    theSQSResponse->open();
  }

  bool
  ListQueuesResponse::next(std::string& aQueueUrl)
  {
    return theSQSResponse->next(aQueueUrl);
  }

  void
  ListQueuesResponse::close()
  {
    return theSQSResponse->close();
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
  SendMessageResponse::getMD5OfMessageBody() const
  {
    return theSQSResponse->getMD5OfMessageBody();
  }

  /**
   * ReceiveMessageResponse
   */
  ReceiveMessageResponse::ReceiveMessageResponse(sqs::ReceiveMessageResponse* r)
    : SQSResponse<sqs::ReceiveMessageResponse>(r) {}

  void
  ReceiveMessageResponse::open()
  {
    theSQSResponse->open();
  }

  bool
  ReceiveMessageResponse::next(Message& aMessage)
  {
    sqs::ReceiveMessageResponse::Message lMessage;
    if (theSQSResponse->next(lMessage)) {
      aMessage.message_body   = lMessage.message_body;
      aMessage.message_size   = lMessage.message_size;
      aMessage.message_md5    = lMessage.message_md5;
      aMessage.message_id     = lMessage.message_id;
      aMessage.meta_data      = lMessage.meta_data;
      aMessage.receipt_handle = lMessage.receipt_handle;
      return true;
    } else {
      return false;
    }
  }

  void
  ReceiveMessageResponse::close()
  {
    theSQSResponse->close();
  }

  int
  ReceiveMessageResponse::getNumberOfRetrievedMessages() const
  {
    return theSQSResponse->getNumberOfRetrievedMessages();
  }

  /**
   * DeleteMessageResponse
   */
  DeleteMessageResponse::DeleteMessageResponse(sqs::DeleteMessageResponse* r)
    : SQSResponse<sqs::DeleteMessageResponse>(r) {}

} /* namespace aws */

