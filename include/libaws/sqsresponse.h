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
      class PeekMessageResponse;
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
  
#if 0
  
  class DeleteQueueResponse : public SQSResponse<sqs::DeleteQueueResponse>
  {
    public:
      virtual ~DeleteQueueResponse();
      
      const std::string&
      getQueueUrl();
  
    protected:
      friend class SQSConnectionImpl;
      DeleteQueueResponse(sqs::DeleteQueueResponse*);
  };
  
  class ListQueuesResponse : public SQSResponse<sqs::ListQueuesResponse>
  {    
    public:
      virtual ~ListQueuesResponse();
      
      const std::string&
      getQueuePrefix();
        
    protected:
      friend class SQSConnectionImpl;
      ListQueuesResponse(sqs::ListQueuesResponse*);
  };
  
  class SendMessageResponse : public SQSResponse<sqs::SendMessageResponse>
  {
    public:
      virtual ~SendMessageResponse();
      
      const std::string&
      getMessageId();

      const std::string&
      getQueueName();
            
    protected:
      friend class SQSConnectionImpl;
      SendMessageResponse(sqs::SendMessageResponse*);
  };
  
  class PeekMessageResponse : public SQSResponse<sqs::PeekMessageResponse>
  {
    public:
      ~PeekMessageResponse();
      
      const std::string&
      getMessageId();

      const std::string&
      getQueueName();

      const char*
      getContent();

      int
      getSize();
        
    protected:
      friend class SQSConnectionImpl;
      PeekMessageResponse(sqs::PeekMessageResponse8r*);
  };
  
  class ReceiveMessageResponse : public SQSResponse<sqs::ReceiveMessageResponse>
  {
    public:
      ~ReceiveMessageResponse();
      
      const std::string&
      getQueueName();

      int
      getNumberOfMessages();

      int
      getVisibilityTimeout();

      int
      getNumberOfRetrievedMessages();

      bool
      isEmpty();
      
    protected:
      friend class SQSConnectionImpl;
      ReceiveMessageResponse(sqs::ReceiveMessageResponse*);
  };
  
  class DeleteMessageResponse : public SQSResponse
  {
    public:
      ~DeleteMessageResponse();
      
      const std::string&
      getQueueName();

      const std::string&
      getMessageId();
  
    protected:
      friend class SQSConnectionImpl;
      DeleteMessageResponse(sqs::DeleteMessageResponse*);
  };
#endif

} /* namespace aws */
#endif
