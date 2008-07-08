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

    class CreateQueueHandler : public SimpleQueryCallBack
    {
      private:
        CreateQueueResponse* theCreateQueueResponse;

        enum States {
          Code        = 1,
          Message     = 2,
          RequestId   = 4,
          HostId      = 8,
          QueueUrl    = 16
        };

      public:
        virtual void
        startElement ( const std::string& localname, int nb_attributes, const xmlChar ** attributes ) ;
        
        virtual void
        characters ( const std::string& value );
        
        virtual void
        endElement ( const std::string& localname );

    };


  } /* namespace sqs  */
} /* namespace aws */

#endif        /* !AWS_SQS_SQSCONNECTION_H */
