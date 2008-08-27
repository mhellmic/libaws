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
#ifndef AWS_SQS_SQSHANDLER_H
#define AWS_SQS_SQSHANDLER_H

#include "awsquerycallback.h"

namespace aws {
  namespace sqs  {

    class CreateQueueResponse;
    class DeleteQueueResponse;
    class ListQueuesResponse;
    class SendMessageResponse;
    class ReceiveMessageResponse;
    class DeleteMessageResponse;

    class QueueErrorHandler : public SimpleQueryCallBack{
      
      
      public:
        enum States {
          ERROR_Code        = 1,
          ERROR_Message     = 2,
          RequestId   			= 4,
          HostId      			= 8,
          QueueUrl    			= 16,
          MessageId   			= 32,
          MD5OfMessageBody 	= 64,
          ReceiptHandle			= 128,
          Body							= 256,
          MetaData          = 512
        };

        virtual void startElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes );
        virtual void characters ( const xmlChar *  value, int len );
        virtual void endElement ( const xmlChar *  localname );

        virtual void responseStartElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes ) = 0;
        virtual void responseCharacters ( const xmlChar *  value, int len ) = 0;
        virtual void responseEndElement ( const xmlChar *  localname ) = 0;

    };

    class CreateQueueHandler : public QueueErrorHandler
    {
      protected:
        friend class SQSConnection;
        CreateQueueResponse* theCreateQueueResponse;

      public:
        virtual void responseStartElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes );
        virtual void responseCharacters ( const xmlChar *  value, int len );
        virtual void responseEndElement ( const xmlChar *  localname );

    };

    class DeleteQueueHandler : public QueueErrorHandler
    {
      protected:
        friend class SQSConnection;
        DeleteQueueResponse* theDeleteQueueResponse;

      public:
        virtual void responseStartElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes );
        virtual void responseCharacters ( const xmlChar *  value, int len );
        virtual void responseEndElement ( const xmlChar *  localname );

    };

    class ListQueuesHandler : public QueueErrorHandler
    {
      protected:
        friend class SQSConnection;
        ListQueuesResponse* theListQueuesResponse;

      public:
        virtual void responseStartElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes );
        virtual void responseCharacters ( const xmlChar *  value, int len );
        virtual void responseEndElement ( const xmlChar *  localname );

    };

    class SendMessageHandler : public QueueErrorHandler
    {
      protected:
        friend class SQSConnection;
        SendMessageResponse* theSendMessageResponse;

      public:
        virtual void responseStartElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes );
        virtual void responseCharacters ( const xmlChar *  value, int len );
        virtual void responseEndElement ( const xmlChar *  localname );

    };

    class ReceiveMessageHandler : public QueueErrorHandler
    {
      private:
        std::string theBody;
      protected:
        friend class SQSConnection;
        ReceiveMessageResponse* theReceiveMessageResponse;

      public:
        virtual void responseStartElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes );
        virtual void responseCharacters ( const xmlChar *  value, int len );
        virtual void responseEndElement ( const xmlChar *  localname );

    };

    class DeleteMessageHandler : public QueueErrorHandler
    {
      protected:
        friend class SQSConnection;
        DeleteMessageResponse* theDeleteMessageResponse;

      public:
        virtual void responseStartElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes );
        virtual void responseCharacters ( const xmlChar *  value, int len );
        virtual void responseEndElement ( const xmlChar *  localname );

    };


  } /* namespace sqs  */
} /* namespace aws */

#endif        /* !AWS_SQS_SQSCONNECTION_H */
