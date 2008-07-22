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
#include "sqs/sqshandler.h"
#include "sqs/sqsresponse.h"
#include <iostream>
#include "awsconnection.h"

#include <string>

using namespace aws;

namespace aws {
  namespace sqs {

    void
    QueueErrorHandler::startElement ( const xmlChar *  localname,
                                      int nb_attributes,
                                      const xmlChar ** attributes ) {
      if (xmlStrEqual ( localname, BAD_CAST "ErrorResponse" ) ) {
        theIsSuccessful = false;
        theQueryErrorResponse = QueryErrorResponse();
      } else if (theIsSuccessful ) {
        responseStartElement ( localname, nb_attributes, attributes );
      } else if (xmlStrEqual ( localname, BAD_CAST "Code" )) {
        setState ( ERROR_Code );
      } else if (xmlStrEqual ( localname, BAD_CAST "Message" )) {
        setState ( ERROR_Message );
      } else if (xmlStrEqual ( localname, BAD_CAST "RequestID" )) {
        setState ( RequestId );
      }

    }

    void QueueErrorHandler::characters ( const xmlChar *  value,
                                         int len ) {

      if (theIsSuccessful ) {
        responseCharacters ( value, len );
      } else {
        std::string lStrValue(( const char* ) value, len);

        if (isSet ( ERROR_Code)) {
          theQueryErrorResponse.setErrorCode(lStrValue);
        } else if (isSet ( ERROR_Message)) {
          theQueryErrorResponse.setErrorMessage(lStrValue);
        } else if (isSet ( RequestId)) {
          theQueryErrorResponse.setRequestId(lStrValue);
        }
      }
    }

    void
    QueueErrorHandler::endElement ( const xmlChar *  localname ) {
      responseEndElement ( localname );
    }

    void
    CreateQueueHandler::responseStartElement ( const xmlChar * localname, int nb_attributes, const xmlChar ** attributes )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "CreateQueueResponse" ) ) {
        theCreateQueueResponse = new CreateQueueResponse();
      } else if ( xmlStrEqual ( localname, BAD_CAST "QueueUrl" ) ) {
        setState ( QueueUrl );
      }
    }

    void
    CreateQueueHandler::responseCharacters ( const xmlChar *  value, int len )
    {
      //std::string test((const char*)value, len);
      //std::cout << "value:" << test << std::endl;
      if ( isSet ( QueueUrl ) ) {
        theCreateQueueResponse->theQueueURL.append ( ( const char* ) value, len );
      }
    }

    void
    CreateQueueHandler::responseEndElement ( const xmlChar * localname )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "QueueUrl" ) ) {
        unsetState ( QueueUrl );
      }
    }

    void
    DeleteQueueHandler::responseStartElement ( const xmlChar * localname, int nb_attributes, const xmlChar ** attributes )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "DeleteQueueResponse" ) ) {
        theDeleteQueueResponse = new DeleteQueueResponse();
      }
    }

    void
    DeleteQueueHandler::responseCharacters ( const xmlChar *  value, int len )
    {
    }

    void
    DeleteQueueHandler::responseEndElement ( const xmlChar * localname )
    {
    }

    void
    ListQueuesHandler::responseStartElement ( const xmlChar * localname, int nb_attributes, const xmlChar ** attributes )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "ListQueuesResponse" ) ) {
        theListQueuesResponse = new ListQueuesResponse();
      } else if ( xmlStrEqual ( localname, BAD_CAST "QueueUrl" ) ) {
        setState ( QueueUrl );
      }
    }

    void
    ListQueuesHandler::responseCharacters ( const xmlChar *  value, int len )
    {
    	if ( isSet ( QueueUrl ) ) {
    		std::string lQueueURL((const char*)value, len);
    		theListQueuesResponse->theQueues.push_back( lQueueURL );
    	}
    }

    void
    ListQueuesHandler::responseEndElement ( const xmlChar * localname )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "QueueUrl" ) ) {
        unsetState ( QueueUrl );
      }
    }

    void
    SendMessageHandler::responseStartElement ( const xmlChar * localname, int nb_attributes, const xmlChar ** attributes )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "SendMessageResponse" ) ) {
      	theSendMessageResponse = new SendMessageResponse();
      } else if ( xmlStrEqual ( localname, BAD_CAST "MessageId" ) ) {
        setState ( MessageId );
      } else if ( xmlStrEqual ( localname, BAD_CAST "MD5OfMessageBody" ) ) {
        setState ( MD5OfMessageBody );
      }
    }

    void
    SendMessageHandler::responseCharacters ( const xmlChar *  value, int len )
    {
    	if ( isSet ( MessageId ) ) {
    		theSendMessageResponse->theMessageId.append( (const char*)value, len );
    	} else if ( isSet ( MD5OfMessageBody )) {
    		theSendMessageResponse->theMD5OfMessageBody.append( (const char*)value, len );
    	}
    }

    void
    SendMessageHandler::responseEndElement ( const xmlChar * localname )
    {
    	if ( xmlStrEqual ( localname, BAD_CAST "MessageId" ) ) {
    		unsetState ( MessageId );
    	} else if ( xmlStrEqual ( localname, BAD_CAST "MD5OfMessageBody" ) ) {
    		unsetState ( MD5OfMessageBody );
    	}
    }

    void
    ReceiveMessageHandler::responseStartElement ( const xmlChar * localname, int nb_attributes, const xmlChar ** attributes )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "ReceiveMessageResponse" ) ) {
      	theReceiveMessageResponse = new ReceiveMessageResponse();
      } else if ( xmlStrEqual ( localname, BAD_CAST "Message" ) ) {
      	ReceiveMessageResponse::Message lMessage;
      	theReceiveMessageResponse->theMessages.push_back(lMessage);
      } else if ( xmlStrEqual ( localname, BAD_CAST "MessageId" ) ) {
        setState ( MessageId );
      } else if ( xmlStrEqual ( localname, BAD_CAST "ReceiptHandle" ) ) {
        setState ( ReceiptHandle );
      } else if ( xmlStrEqual ( localname, BAD_CAST "MD5OfMessageBody" ) ) {
        setState ( MD5OfMessageBody );
      } else if ( xmlStrEqual ( localname, BAD_CAST "Body" ) ) {
        setState ( Body );
      }
    }

    void
    ReceiveMessageHandler::responseCharacters ( const xmlChar *  value, int len )
    {
    	if ( isSet ( MessageId ) ) {
    		ReceiveMessageResponse::Message& lMessage = theReceiveMessageResponse->theMessages.back();
    		std::string lMessageId((const char*)value, len);
    		lMessage.message_id = lMessageId;
    	} else if ( isSet ( ReceiptHandle )) {
    		ReceiveMessageResponse::Message& lMessage = theReceiveMessageResponse->theMessages.back();
    		std::string lReceiptHandle((const char*)value, len);
    		lMessage.receipt_handle = lReceiptHandle;
    	} else if ( isSet ( MD5OfMessageBody )) {
    		ReceiveMessageResponse::Message& lMessage = theReceiveMessageResponse->theMessages.back();
    		std::string lMessageMD5((const char*)value, len);
    		lMessage.message_md5 = lMessageMD5;
    	} else if ( isSet ( Body )) {
    		ReceiveMessageResponse::Message& lMessage = theReceiveMessageResponse->theMessages.back();
    		// transfering ownership of the message body to the message
    		lMessage.message_body = AWSConnection::base64Decode((const char*) value, len, lMessage.message_size);
    	}
    }

    void
    ReceiveMessageHandler::responseEndElement ( const xmlChar * localname )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "MessageId" ) ) {
      	unsetState ( MessageId );
      } else if ( xmlStrEqual ( localname, BAD_CAST "ReceiptHandle" ) ) {
      	unsetState ( ReceiptHandle );
      } else if ( xmlStrEqual ( localname, BAD_CAST "MD5OfMessageBody" ) ) {
      	unsetState ( MD5OfMessageBody );
      } else if ( xmlStrEqual ( localname, BAD_CAST "Body" ) ) {
      	unsetState ( Body );
      }
    }

    void
    DeleteMessageHandler::responseStartElement ( const xmlChar * localname, int nb_attributes, const xmlChar ** attributes )
    {
      if ( xmlStrEqual ( localname, BAD_CAST "DeleteMessageResponse" ) ) {
      	theDeleteMessageResponse = new DeleteMessageResponse();
      }
    }

    void
    DeleteMessageHandler::responseCharacters ( const xmlChar *  value, int len )
    {
    }

    void
    DeleteMessageHandler::responseEndElement ( const xmlChar * localname )
    {
    }

  } /* namespace sqs  */
} /* namespace aws */
