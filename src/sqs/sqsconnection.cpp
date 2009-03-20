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

#include "sqs/sqsconnection.h"
#include "sqs/sqsresponse.h"
#include "sqs/sqshandler.h"

#include <sstream>
#include <memory>
#include <cassert>


using namespace aws;

namespace aws { namespace sqs {

  const std::string SQSConnection::DEFAULT_VERSION = "2008-01-01";
  const std::string SQSConnection::DEFAULT_HOST = "queue.amazonaws.com";

  SQSConnection::SQSConnection(const std::string& aAccessKeyId,
                               const std::string& aSecretAccessKey,
                               const std::string& aCustomHost )
  : AWSQueryConnection(aAccessKeyId, aSecretAccessKey, aCustomHost.size()==0?DEFAULT_HOST:aCustomHost,
                       DEFAULT_VERSION, 80, true)
  {

  }

  SQSConnection::SQSConnection(const std::string& aAccessKeyId,
															 const std::string& aSecretAccessKey,
															 const std::string& aCustomHost,
															 int aPort, bool aIsSecure)
	: AWSQueryConnection(aAccessKeyId, aSecretAccessKey, aCustomHost.size()==0?DEFAULT_HOST:aCustomHost, 
                       DEFAULT_VERSION, aPort, aIsSecure)
	{

	}

  CreateQueueResponse*
  SQSConnection::createQueue ( const std::string &aQueueName, int aDefaultVisibilityTimeout)
  {
    ParameterMap lMap;
    lMap.insert ( ParameterPair ( "QueueName", aQueueName ) );

    if (aDefaultVisibilityTimeout > -1 ) {
      std::stringstream s;
      s << aDefaultVisibilityTimeout;
      lMap.insert ( ParameterPair ( "DefaultVisibilityTimeout", s.str() ) );
    }

    CreateQueueHandler lHandler;
    makeQueryRequest ( "CreateQueue", &lMap, &lHandler );
    if (lHandler.isSuccessful()) {
      setCommons(lHandler, lHandler.theCreateQueueResponse);
      return lHandler.theCreateQueueResponse;
    } else {
    	throw CreateQueueException( lHandler.getQueryErrorResponse() );
    }
  }

  DeleteQueueResponse*
  SQSConnection::deleteQueue(const std::string &aQueueUrl)
  {
    ParameterMap lMap;

    DeleteQueueHandler lHandler;
    makeQueryRequest ( aQueueUrl,  "DeleteQueue", &lMap, &lHandler );
    if (lHandler.isSuccessful()) {
      setCommons(lHandler, lHandler.theDeleteQueueResponse);
      return lHandler.theDeleteQueueResponse;
    } else {
    	throw DeleteQueueException( lHandler.getQueryErrorResponse() );
    }
  }

  ListQueuesResponse*
  SQSConnection::listQueues(const std::string &aQueueNamePrefix)
  {
    ParameterMap lMap;
  	if (aQueueNamePrefix != "") {
      lMap.insert ( ParameterPair ( "QueueNamePrefix", aQueueNamePrefix ) );
  	}
  	ListQueuesHandler lHandler;
    makeQueryRequest ( "ListQueues", &lMap, &lHandler );
    if (lHandler.isSuccessful()) {
      setCommons(lHandler, lHandler.theListQueuesResponse);
      return lHandler.theListQueuesResponse;
    } else {
    	throw ListQueuesException( lHandler.getQueryErrorResponse() );
    }
  }

  SendMessageResponse*
  SQSConnection::sendMessage(const std::string &aQueueUrl, const std::string &aMessageBody)
  {
    ParameterMap lMap;
    long lBody64Len;
    std::string enc = AWSConnection::base64Encode(aMessageBody.c_str(), aMessageBody.size(), lBody64Len);
    if (enc.size() > 32768) {
      std::stringstream lTmp;
      lTmp << "Message larger than 32kB : " << enc.size() / 1024 << " kb";
      throw SendMessageException( QueryErrorResponse("1", lTmp.str(), "", "") );
    }
    lMap.insert ( ParameterPair ( "MessageBody", enc ) );
    return sendMessage(aQueueUrl, lMap);
  }
    
  SendMessageResponse*
  SQSConnection::sendMessage (const std::string &aQueueUrl,
                              ParameterMap& lMap) {
    SendMessageHandler lHandler;
    makeQueryRequest (aQueueUrl, "SendMessage", &lMap, &lHandler);
    if (lHandler.isSuccessful()) {
      setCommons(lHandler, lHandler.theSendMessageResponse);
        return lHandler.theSendMessageResponse;
      } else {
        throw SendMessageException (lHandler.getQueryErrorResponse());
      }
  }

  ReceiveMessageResponse*
  SQSConnection::receiveMessage (const std::string &aQueueUrl,
                                 int aNumberOfMessages,
                                 int aVisibilityTimeout) {
    ParameterMap lMap;
    if (aNumberOfMessages != 0) {
        std::stringstream s;
        s << aNumberOfMessages;
        lMap.insert (ParameterPair ("MaxNumberOfMessages", s.str()));
      }
    if (aVisibilityTimeout > -1) {
        std::stringstream s;
        s << aVisibilityTimeout;
        lMap.insert (ParameterPair ("VisibilityTimeout", s.str()));
      }
  
    return receiveMessage (aQueueUrl, lMap);
  } 
  
  ReceiveMessageResponse*
  SQSConnection::receiveMessage (const std::string &aQueueUrl,
                                 ParameterMap& lMap) {
    ReceiveMessageHandler lHandler;
    makeQueryRequest (aQueueUrl, "ReceiveMessage", &lMap, &lHandler);
    if (lHandler.isSuccessful()) {
      setCommons(lHandler, lHandler.theReceiveMessageResponse);
        return lHandler.theReceiveMessageResponse;
      } else {
        throw ReceiveMessageException (lHandler.getQueryErrorResponse());
      }
  }

  DeleteMessageResponse*
  SQSConnection::deleteMessage(const std::string &aQueueUrl, const std::string &aReceiptHandle)
  {
    ParameterMap lMap;
    lMap.insert ( ParameterPair ( "ReceiptHandle", aReceiptHandle ) );

    DeleteMessageHandler lHandler;
    makeQueryRequest ( aQueueUrl, "DeleteMessage", &lMap, &lHandler );
    if (lHandler.isSuccessful()) {
      setCommons(lHandler, lHandler.theDeleteMessageResponse);
      return lHandler.theDeleteMessageResponse;
    } else {
    	throw DeleteMessageException( lHandler.getQueryErrorResponse() );
    }
  }

}}//namespaces

