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

#include <openssl/hmac.h>


#include "awsquerycallback.h"


namespace aws {

  void SimpleQueryCallBack::startElementNs ( const xmlChar * localname,
      const xmlChar * prefix,
      const xmlChar * URI,
      int nb_namespaces,
      const xmlChar ** namespaces,
      int nb_attributes,
      int nb_defaulted,
      const xmlChar ** attributes ) {
     startElement ( localname, nb_attributes, attributes );
  }

  void SimpleQueryCallBack::charactersSAXFunc ( const xmlChar * value,
      int len ) {;
    characters ( value, len );

  }

  void SimpleQueryCallBack::endElementNs ( const xmlChar * localname,
      const xmlChar * prefix,
      const xmlChar * URI ) {
    endElement ( localname );
  }
  
  void QueryCallBack::SAX_StartElementNs ( void * ctx,
      const xmlChar * localname,
      const xmlChar * prefix,
      const xmlChar * URI,
      int nb_namespaces,
      const xmlChar ** namespaces,
      int nb_attributes,
      int nb_defaulted,
      const xmlChar ** attributes ) {
    QueryCallBack* lCallBack  = static_cast<QueryCallBack*> ( ctx );
    lCallBack->startElementNs ( localname, prefix, URI, nb_namespaces, namespaces, nb_attributes, nb_defaulted, attributes );
  }

  void QueryCallBack::SAX_CharactersSAXFunc ( void * ctx,
      const xmlChar * value,
      int len ) {
    QueryCallBack* lCallBack  = static_cast<QueryCallBack*> ( ctx );
    lCallBack->charactersSAXFunc( value, len );
  }

  void QueryCallBack::SAX_EndElementNs ( void * ctx,
      const xmlChar * localname,
      const xmlChar * prefix,
      const xmlChar * URI ) {
    QueryCallBack* lCallBack  = static_cast<QueryCallBack*> ( ctx );
    lCallBack->endElementNs ( localname, prefix, URI );
  }

}//namespace
