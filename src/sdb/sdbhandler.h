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
#ifndef AWS_SDB_SDBHANDLER_H
#define AWS_SDB_SDBHANDLER_H

#include "awsquerycallback.h"

namespace aws {
  namespace sdb  {

    class SDBResponse;
    class CreateDomainResponse;
    
    template <class T>
    class SDBHandler : public SimpleQueryCallBack{
      friend class SDBConnection;
      
      protected:               
      T* theResponse;
      
      public:
        enum States {
          ERROR_Code        = 1,
          ERROR_Message     = 2,
          RequestId   = 4,
          HostId      = 8,
          BoxUsage    = 16
        };
        
        virtual void startElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes );
        virtual void characters ( const xmlChar *  value, int len );
        virtual void endElement ( const xmlChar *  localname );
        
        virtual void responseStartElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes ) = 0;
        virtual void responseCharacters ( const xmlChar *  value, int len ) = 0;
        virtual void responseEndElement ( const xmlChar *  localname ) = 0; 
      
    };

    class CreateDomainHandler : public SDBHandler<CreateDomainResponse>
    {
      protected:

      public:
        virtual void responseStartElement ( const xmlChar *  localname, int nb_attributes, const xmlChar ** attributes );
        virtual void responseCharacters ( const xmlChar *  value, int len );
        virtual void responseEndElement ( const xmlChar *  localname );

    };


  } /* namespace sdb  */
} /* namespace aws */

#endif  
