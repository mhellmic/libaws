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

#include "sqs/sqsresponse.h"


using namespace aws;

namespace aws {
  namespace sqs {

    const std::string&
    CreateQueueResponse::getQueueUrl() const
    {
      return theQueueURL;
    }

    void
    ListQueuesResponse::open()
    {
      theIterator = theQueues.begin();
    }

    bool
    ListQueuesResponse::next(std::string& aQueueUrl)
    {
      if (theIterator != theQueues.end()) {
        aQueueUrl = *theIterator;
        ++theIterator;
        return true;
      } else {
        return false;
      }
    }

    void
    ListQueuesResponse::close()
    {
      theIterator = theQueues.begin();
    }

    const std::string&
    SendMessageResponse::getMessageId() const
    {
      return theMessageId;
    }

    const std::string&
    SendMessageResponse::getMD5OfMessageBody() const
    {
      return theMD5OfMessageBody;
    }

    ReceiveMessageResponse::~ReceiveMessageResponse()
    {
    	for (std::vector<Message>::iterator lIter = theMessages.begin();
					 lIter != theMessages.end(); ++lIter)
    	{
    		delete[] (*lIter).message_body;
    	}
    }

    void
    ReceiveMessageResponse::open()
    {
      theIterator = theMessages.begin();
    }

    bool
    ReceiveMessageResponse::next(Message& aMessage)
    {
      if (theIterator != theMessages.end()) {
        aMessage = *theIterator;
        ++theIterator;
        return true;
      } else {
        return false;
      }
    }

    void
    ReceiveMessageResponse::close()
    {
      theIterator = theMessages.end();
    }

    int
    ReceiveMessageResponse::getNumberOfRetrievedMessages() const
    {
      return theMessages.size();
    }

  } /* namespace sqs */
} /* namespace aws */
