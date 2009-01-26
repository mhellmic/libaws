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
#include <fstream>
#include <stdlib.h>
#include <libaws/aws.h>

using namespace aws;

bool
listQueues (SQSConnectionPtr aSQS, std::string aQueueNamePrefix = "") {
  try {
      ListQueuesResponsePtr lRes = aSQS->listQueues (aQueueNamePrefix);
      std::cout << "queues listed sucessfully" << std::endl;
      lRes->open();
      std::string lQueueURL;
      while (lRes->next (lQueueURL)) {
        std::cout << "   Queue Name: " << lQueueURL << "\n";
        }
      lRes->close();
    } catch (CreateBucketException &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  return true;
}

bool createQueue (SQSConnectionPtr aSQS, std::string aQueueName, int aVisibilityTimeout) {
  try {
      CreateQueueResponsePtr lRes = aSQS->createQueue (aQueueName, aVisibilityTimeout);
      std::cout << "queue created successfully" << std::endl;
      std::cout << "   url: "  << lRes->getQueueUrl() << std::endl;
    } catch (CreateBucketException &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  return true;
}

bool deleteQueue (SQSConnectionPtr aSQS, std::string aQueueName) {
  try {
      DeleteQueueResponsePtr lRes = aSQS->deleteQueue (aQueueName);
      std::cout << "queue deleted successfully" << std::endl;
    } catch (CreateBucketException &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  return true;
}

bool deleteAllQueues (SQSConnectionPtr aSQS) {
  try {
      std::cout << "start deleting queues:" << std::endl;
      ListQueuesResponsePtr lListQueues = aSQS->listQueues();
      lListQueues->open();
      std::string lQueueURL;
      while (lListQueues->next (lQueueURL)) {
          DeleteQueueResponsePtr lDelete = aSQS->deleteQueue (lQueueURL);
          std::cout << "    deleted queue: " << lQueueURL << std::endl;
        }
      lListQueues->close();
      std::cout << "all queues successfully deleted." << std::endl;
    } catch (ListBucketException &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  return true;
}


bool sendMessage (SQSConnectionPtr aSQS, std::string aQueueName, std::string aMessage) {
  try {
      SendMessageResponsePtr lRes = aSQS->sendMessage (aQueueName, aMessage);
      std::cout << "send message successfully" << std::endl;
      std::cout << "   url: ["  << lRes->getMessageId() << std::endl;
      std::cout << "   md5: ["  << lRes->getMD5OfMessageBody() << std::endl;
    } catch (CreateBucketException &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  return true;
}


bool receiveMessage (SQSConnectionPtr aSQS, std::string aQueueName,  int aMaxNbMessages, int aVisibilityTimeout) {
  try {
      ReceiveMessageResponsePtr lReceiveMessages = aSQS->receiveMessage (aQueueName, aMaxNbMessages, aVisibilityTimeout);
      lReceiveMessages->open();
      ReceiveMessageResponse::Message lMessage;
      std::cout << "received messages:" << std::endl;
      int lNb = 0;
      while (lReceiveMessages->next (lMessage)) {
          std::cout << "  Message Nb. " << lNb++ << std::endl;
          std::cout << "    message-id: [" << lMessage.message_id << "]" << std::endl;
          std::cout << "    message-handle: [" << lMessage.receipt_handle << "]"<< std::endl;
          std::cout << "    message-md5: [" << lMessage.message_md5 << "]" << std::endl;
          std::cout << "    message-size: [" << lMessage.message_size << "]" << std::endl;
          std::cout << "    message-content: [";
          std::cout.write (lMessage.message_body, lMessage.message_size);
          std::cout << "]" << std::endl;
        }
      lReceiveMessages->close();
    } catch (ListBucketException &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  return true;
}

bool deleteMessage (SQSConnectionPtr aSQS, std::string aQueueName, std::string aReceiptHandle) {
  try {
      DeleteMessageResponsePtr lRes = aSQS->deleteMessage (aQueueName, aReceiptHandle);
      std::cout << "deleted message successfully" << std::endl;
    } catch (CreateBucketException &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  return true;
}

bool deleteAllMessages (SQSConnectionPtr aSQS, std::string aQueueName) {
  try {
      bool lMessagesReceived = true;
      ReceiveMessageResponse::Message lMessage;
      ReceiveMessageResponsePtr lReceiveMessages;

      while (lMessagesReceived) {
          lMessagesReceived = false;
          lReceiveMessages = aSQS->receiveMessage (aQueueName);
          lReceiveMessages->open();
          std::cout << "start deleting messages:" << std::endl;
          while (lReceiveMessages->next (lMessage)) {
              lMessagesReceived = true;
              std::cout << "   message-id: [" << lMessage.message_id  << "]" << std::endl;
              DeleteMessageResponsePtr lDelRes = aSQS->deleteMessage (aQueueName, lMessage.receipt_handle);
            }
          lReceiveMessages->close();
        }

      std::cout << "deleted messages successfully" << std::endl;
    } catch (CreateBucketException &e) {
      std::cerr << e.what() << std::endl;
      return false;
    }
  return true;
}

int
main (int argc, char** argv) {
  char* lAction = 0;
  char* lQueueName = 0;
  char* lPrefix = 0;
  int   lMaxNbMessages = -1;
  char* lMessage = 0;
  int   lVisibilityTimeOut = -1;
  char* lAccessKeyId = 0;
  char* lSecretAccessKey = 0;
  char* lReceiptHandle = 0;
  char* lHost = 0;

  int c;
  opterr = 0;

  AWSConnectionFactory* lFactory = AWSConnectionFactory::getInstance();

  while ( (c = getopt (argc, argv, "hi:s:a:n:p:x:m:r:")) != -1)
    switch (c) {
      case 'i':
        lAccessKeyId = optarg;
        break;
      case 's':
        lSecretAccessKey = optarg;
        break;
      case 'a':
        lAction = optarg;
        break;
      case 'o':
      	lHost = optarg;
      	break;
      case 'n':
        lQueueName = optarg;
        break;
      case 'p':
        lPrefix = optarg;
        break;
      case 'x':
        lMaxNbMessages = atoi (optarg);
        break;
      case 'm':
        lMessage = optarg;
        break;
      case 'v':
        lVisibilityTimeOut = atoi (optarg);
        break;
      case 'r':
        lReceiptHandle = optarg;
        break;
      case 'h': {
          std::cout << "libaws version " << lFactory->getVersion() << std::endl;
          std::cout << "Usage: sqs <options>" << std::endl;
          std::cout << "  -i: AWS Access Key Id"  << std::endl;
          std::cout << "  -s: AWS Secret Access Key"  << std::endl;
          std::cout << "  -a action: Action to perform" << std::endl;
          std::cout << "             list-queues: List all queues" << std::endl;
          std::cout << "             create-queue: Create a queue" << std::endl;
          std::cout << "             delete-queue: Delete a queue" << std::endl;
          std::cout << "             delete-all-queues: Delete all queues" << std::endl;
          std::cout << "             send-message: Send a message" << std::endl;
          std::cout << "             list-message: Receive messages" << std::endl;
          std::cout << "             delete-message: Delete messages" << std::endl;
          std::cout << "             delete-all-messages: Delete all messages retrieved" << std::endl;
          std::cout << "  -o host name"  << std::endl;
          std::cout << "  -n name: A Queue Name/URL"  << std::endl;
          std::cout << "  -p prefix: Prefix for listing queues"  << std::endl;
          std::cout << "  -x #messages: Number of messages to return"  << std::endl;
          std::cout << "  -m message: the message to send" << std::endl;
          std::cout << "  -v visibility timeout: the visibility timeout" << std::endl;
          std::cout << "  -r receipt-handle: the receipt-handle" << std::endl;
          std::cout << "  -h help: display help" << std::endl;
          exit (1);
        }
      case '?':
        if (isprint (optopt))
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        exit (1);
      default:
        exit (1);
      }

  if (!lAccessKeyId) {
    lAccessKeyId = getenv ("AWS_ACCESS_KEY");
  }

  if (!lSecretAccessKey) {
    lSecretAccessKey = getenv ("AWS_SECRET_ACCESS_KEY");
  }

  if (!lHost) {
  	lHost = getenv ("SQS_HOST");
  }

  if (!lAccessKeyId) {
		std::cerr << "No Access Key given" << std::endl;
		std::cerr << "Either use -i as a command line argument or set AWS_ACCESS_KEY as an environmental variable" << std::endl;
		exit (1);
	}

  if (!lSecretAccessKey) {
		std::cerr << "No Secret Access Key given" << std::endl;
		std::cerr << "Either use -s as a command line argument or set AWS_SECRET_ACCESS_KEY as an environmental variable" << std::endl;
		exit (1);
	}

  SQSConnectionPtr lSQSRest;
  if (!lHost) {
  	lSQSRest = lFactory->createSQSConnection (lAccessKeyId, lSecretAccessKey);
  }
  else {
    // aPort = -1 -> we have specified our own
    // aIsSecure = false -> we would like http instead of https
  	lSQSRest = lFactory->createSQSConnection(lAccessKeyId, lSecretAccessKey, lHost, -1, false);
  }

  if (!lAction) {
      std::cerr << "No Action parameter specified." << std::endl;
      std::cerr << "Use -a as a command line argument" << std::endl;
      exit (1);
    }
  std::string lActionString (lAction);

  if (lActionString.compare ("list-queues") == 0) {
      listQueues (lSQSRest, lPrefix == 0 ? "" : lPrefix);
    } else if (lActionString.compare ("create-queue") == 0) {
      if (!lQueueName) {
          std::cerr << "No queue name parameter specified." << std::endl;
          std::cerr << "Use -n as a command line argument" << std::endl;
          exit (1);
        }
      createQueue (lSQSRest, lQueueName, lVisibilityTimeOut);
    } else if (lActionString.compare ("delete-queue") == 0) {
      if (!lQueueName) {
          std::cerr << "No queue name parameter specified." << std::endl;
          std::cerr << "Use -n as a command line argument" << std::endl;
          exit (1);
        }
        deleteQueue (lSQSRest, lQueueName);
    } else if (lActionString.compare ("delete-all-queues") == 0) {
      deleteAllQueues (lSQSRest);
    } else if (lActionString.compare ("send-message") == 0) {
      if (!lQueueName) {
          std::cerr << "No queue name parameter specified." << std::endl;
          std::cerr << "Use -n as a command line argument" << std::endl;
          exit (1);
        }
      if (!lMessage) {
          std::cerr << "No message parameter specified." << std::endl;
          std::cerr << "Use -m as a command line argument" << std::endl;
          exit (1);
        }
      sendMessage (lSQSRest, lQueueName, lMessage);
    } else if (lActionString.compare ("list-messages") == 0 || lActionString.compare ("list-message") == 0) {
      if (!lQueueName) {
          std::cerr << "No queue name parameter specified." << std::endl;
          std::cerr << "Use -n as a command line argument" << std::endl;
          exit (1);
        }
      receiveMessage (lSQSRest, lQueueName, lMaxNbMessages, lVisibilityTimeOut);
    } else if (lActionString.compare ("delete-message") == 0) {
      if (!lQueueName) {
          std::cerr << "No queue name parameter specified." << std::endl;
          std::cerr << "Use -n as a command line argument" << std::endl;
          exit (1);
        }
      if (!lReceiptHandle) {
          std::cerr << "No receipt handle parameter specified." << std::endl;
          std::cerr << "Use -r as a command line argument" << std::endl;
          exit (1);
        }
      deleteMessage (lSQSRest, lQueueName, lReceiptHandle);
    } else if (lActionString.compare ("delete-all-messages") == 0) {
      if (!lQueueName) {
          std::cerr << "No queue name parameter specified." << std::endl;
          std::cerr << "Use -n as a command line argument" << std::endl;
          exit (1);
        }
      deleteAllMessages (lSQSRest, lQueueName);
    } else {
      std::cerr << "Command not recognized." << std::endl;
      exit(1);
    }

}

