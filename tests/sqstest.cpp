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
#include <iostream>
#include <sstream>
#include <libaws/aws.h>
#include <stdlib.h>
#include <../src/logging/logging.hh> //HACK 

using namespace aws;

int
testQueues(SQSConnection* lSQSCon)
{
  {
    try {
    	// test create queue
      CreateQueueResponsePtr lCreateQueue = lSQSCon->createQueue("aQueue");
      std::string lAQueueURL = lCreateQueue->getQueueUrl();
      std::cout << "Bucket created successfully. QueueUrl: " << lAQueueURL << std::endl;
      lCreateQueue = lSQSCon->createQueue("bQueue");
      std::string lBQueueURL = lCreateQueue->getQueueUrl();
      std::cout << "Bucket created successfully. QueueUrl: " << lBQueueURL << std::endl;

      // test list queues without prefix
      ListQueuesResponsePtr lListQueues = lSQSCon->listQueues();
      lListQueues->open();
      std::string lName;
      int lCount = 0;
      while (lListQueues->next(lName)) {
        std::cout << "Queue Name: " << lName << std::endl;
        lCount++;
      }
      lListQueues->close();
      if (lCount != 2) {
      	std::cout << "Wrong number of queues (exp. 2): " << lCount << std::endl;
      	return 1;
      }

      // test list queues with prefix
      lCount = 0;
      lListQueues = lSQSCon->listQueues("a");
			lListQueues->open();
			while (lListQueues->next(lName)) {
				std::cout << "Queue Name: " << lName << std::endl;
				lCount++;
			}
			lListQueues->close();
			if (lCount != 1) {
      	std::cout << "Wrong number of queues (exp. 1): " << lCount << std::endl;
				return 1;
			}

			// test delete queue
			DeleteQueueResponsePtr lDeleteQueue = lSQSCon->deleteQueue(lAQueueURL);
      std::cout << "Queue " << lAQueueURL << " has been deleted" << std::endl;
			lDeleteQueue = lSQSCon->deleteQueue(lBQueueURL);
      std::cout << "Queue " << lBQueueURL << " has been deleted" << std::endl;

      // test list queues (should be empty now)
      lCount = 0;
      lListQueues = lSQSCon->listQueues();
			lListQueues->open();
			while (lListQueues->next(lName)) {
				std::cout << "Queue Name: " << lName << std::endl;
				lCount++;
			}
			lListQueues->close();
			if (lCount != 0) {
      	std::cout << "Wrong number of queues (exp. 0): " << lCount << std::endl;
				return 1;
			}

    } catch (CreateBucketException& e) {
      std::cerr << "Couldn't create bucket" << std::endl;
      std::cerr << e.what() << std::endl;
      return 1;
    }
  }
  return 0;
}

int
testMessages(SQSConnection* lSQSCon)
{
  {
    try {
    	// create queue
      CreateQueueResponsePtr lCreateQueue = lSQSCon->createQueue("aQueue");
      std::string lAQueueURL = lCreateQueue->getQueueUrl();
      std::cout << "Bucket created successfully. QueueUrl: " << lAQueueURL << std::endl;

      // send message
      SendMessageResponsePtr lSendResponse = lSQSCon->sendMessage(lAQueueURL, "my cool body");
      std::string lMessageId = lSendResponse->getMessageId();
      if (lMessageId == "") {
      	std::cout << "Emtpy message ID" << std::cout;
      	return 1;
      }
      std::cout << "Message sent. ID: " << lMessageId << std::endl;

      // receive message
      ReceiveMessageResponsePtr lReceiveResponse = lSQSCon->receiveMessage(lAQueueURL,
      		1, -1);
      ReceiveMessageResponse::Message lMessage;
      int lCount = 0;
      lReceiveResponse->open();
      while (lReceiveResponse->next(lMessage)) {
      	std::cout << "Message received. ID: " << lMessage.message_id << std::endl;
      	std::cout << "Message body: ";
      	std::cout.write(lMessage.message_body, lMessage.message_size);
      	std::cout << std::endl;
      	lCount++;
      }
      lReceiveResponse->close();
      if (lCount != 1) {
      	std::cout << "Wrong number of messages (exp. 1): " << lCount << std::endl;
      	return 1;
      }

      // delete message
      DeleteMessageResponsePtr lDeleteResponse = lSQSCon->deleteMessage(lAQueueURL,
      		lMessage.receipt_handle);
      std::cout << "Message with ID " << lMessage.message_id << " has been deleted" << std::endl;

      // delete queue
			DeleteQueueResponsePtr lDeleteQueue = lSQSCon->deleteQueue(lAQueueURL);
      std::cout << "Queue " << lAQueueURL << " has been deleted" << std::endl;

    } catch (CreateBucketException& e) {
      std::cerr << "Couldn't create bucket" << std::endl;
      std::cerr << e.what() << std::endl;
      return 1;
    }
  }
  return 0;
}

int
sqstest(int argc, char** argv)
{

  AWSConnectionFactory* lFactory = AWSConnectionFactory::getInstance();
  //logging::LoggerManager::logmanager()->setLoggerConfig("|1|logging.log");
  //logging::LoggerManager::logmanager()->setFileName ( "", "logging.log" );
  std::cout << "Testing libaws version " << lFactory->getVersion() << std::endl;

  char* lAccessKeyId = getenv("AWS_ACCESS_KEY");
  char* lSecretAccessKey = getenv("AWS_SECRET_ACCESS_KEY");
  if (lAccessKeyId == 0 || lSecretAccessKey == 0) {
    std::cerr << "Environment variables (i.e. AWS_ACCESS_KEY or AWS_SECRET_ACCESS_KEY) not set"
        << std::endl;
    return 1;
  }

  SQSConnectionPtr lS3Rest;
  char* lHost = getenv("SQS_HOST");
  if (lHost == 0) {
  	lS3Rest = lFactory->createSQSConnection(lAccessKeyId, lSecretAccessKey);
  }
  else {
    // aPort = -1 -> we have specified our own
    // aIsSecure = false -> we would like http instead of https
  	lS3Rest = lFactory->createSQSConnection(lAccessKeyId, lSecretAccessKey, lHost, -1, false);
  }

  int lReturnCode;
  try {
   // lReturnCode = testQueues(lS3Rest.get());
   // if (lReturnCode != 0)
   //		return lReturnCode;

    lReturnCode = testMessages(lS3Rest.get());
    if (lReturnCode != 0)
      return lReturnCode;


  } catch (AWSConnectionException& e) {
    std::cerr << e.what() << std::endl;
    return 2;
  }

  lFactory->shutdown();

  return 0;
}
