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

#include <libaws/sqsexception.h>
#include "sqs/sqsresponse.h"

namespace aws {

    SQSException::SQSException (const QueryErrorResponse& aError) {
      theErrorMessage = aError.getErrorMessage();
      theOrigErrorCode = aError.getErrorCode();
      theErrorCode = parseError(theOrigErrorCode);
      theRequestId  = aError.getRequestId();
    }

    SQSException::~SQSException() throw() {}

    const char*
    SQSException::what() const throw() {
      return theErrorMessage.c_str();
    }

    SQSException::ErrorCode
        SQSException::parseError (const std::string& aString) {
      if (aString.compare ("InvalidAccessKeyId") == 0) {
        return SQSException::InvalidAccessKeyId;
      }else if (aString.compare ("ReadCountOutOfRange") == 0) {
        return SQSException::ReadCountOutOfRange;
      } else if (aString.compare ("AWS.SimpleQueueService.QueueDeletedRecently") == 0) {
        return SQSException::AWS_SimpleQueueService_QueueDeletedRecently;
      } else if (aString.compare ("AWS.SimpleQueueService.QueueNameExists") == 0 || aString.compare ("QueueAlreadyExists") == 0) {
        return SQSException::AWS_SimpleQueueService_QueueNameExists;
      } else {
        return SQSException::Unknown;
      }
    }

    CreateQueueException::CreateQueueException (const QueryErrorResponse& aError)
        : SQSException (aError) {}

    CreateQueueException::~CreateQueueException() throw() {}

    ListQueuesException::ListQueuesException (const QueryErrorResponse& aError)
        : SQSException (aError) {}

    ListQueuesException::~ListQueuesException() throw() {}

    DeleteQueueException::DeleteQueueException (const QueryErrorResponse& aError)
        : SQSException (aError) {}

    DeleteQueueException::~DeleteQueueException() throw() {}

    SendMessageException::SendMessageException (const QueryErrorResponse& aError)
        : SQSException (aError) {}

    SendMessageException::~SendMessageException() throw() {}

    ReceiveMessageException::ReceiveMessageException (const QueryErrorResponse& aError)
        : SQSException (aError) {}

    ReceiveMessageException::~ReceiveMessageException() throw() {}

    DeleteMessageException::DeleteMessageException (const QueryErrorResponse& aError)
        : SQSException (aError) {}

    DeleteMessageException::~DeleteMessageException() throw() {}

  } /* namespace aws */
