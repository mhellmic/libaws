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
#ifndef AWS_AWSQUERYCALLBACK_H
#define AWS_AWSQUERYCALLBACK_H

#include <libxml/parser.h>

namespace aws
{

  class SimpleQueryCallBack : QueryCallBack {

    protected:
      void setState(uint64_t s)   { theCurrentState |= s; }
      bool isSet(uint64_t s)      { return theCurrentState & s; }
      void unsetState(uint64_t s) { theCurrentState ^= s; }
    
      uint64_t theCurrentState;
    
    protected:
      SimpleQueryCallBack() : QueryCallBack() {};

    public:
      void startElementNs ( const xmlChar * localname,
                          const xmlChar * prefix,
                          const xmlChar * URI,
                          int nb_namespaces,
                          const xmlChar ** namespaces,
                          int nb_attributes,
                          int nb_defaulted,
                          const xmlChar ** attributes );

      void charactersSAXFunc ( const xmlChar * value,
                             int len );

      void endElementNs ( const xmlChar * localname,
                        const xmlChar * prefix,
                        const xmlChar * URI );

      virtual void startElement ( const std::string& localname, int nb_attributes, const xmlChar ** attributes ) =0;
      virtual void characters ( const std::string& value ) =0;
      virtual void endElement ( const std::string& localname ) =0;

  };

  class QueryCallBack{

    public:
      enum Status {
        Unknown,
        Error,
        Successful
      };

    protected:
      std::vector<Error> theErrors;
      Status theStatus;

    public:

      QueryCallBack() :theStatus ( Status.Unknown ) {};



      virtual void startElementNs ( const xmlChar * localname,
                                    const xmlChar * prefix,
                                    const xmlChar * URI,
                                    int nb_namespaces,
                                    const xmlChar ** namespaces,
                                    int nb_attributes,
                                    int nb_defaulted,
                                    const xmlChar ** attributes ) =0;

      virtual void charactersSAXFunc ( const xmlChar * value,
                                       int len ) =0;

      virtual void endElementNs ( const xmlChar * localname,
                                  const xmlChar * prefix,
                                  const xmlChar * URI ) =0;

    public:
      static void SAX_StartElementNs ( void * ctx,
                                       const xmlChar * localname,
                                       const xmlChar * prefix,
                                       const xmlChar * URI,
                                       int nb_namespaces,
                                       const xmlChar ** namespaces,
                                       int nb_attributes,
                                       int nb_defaulted,
                                       const xmlChar ** attributes );

      static void SAX_CharactersSAXFunc ( void * ctx,
                                          const xmlChar * value,
                                          int len );

      static void SAX_EndElementNs ( void * ctx,
                                     const xmlChar * localname,
                                     const xmlChar * prefix,
                                     const xmlChar * URI );
  };


}//namespace

#endif
