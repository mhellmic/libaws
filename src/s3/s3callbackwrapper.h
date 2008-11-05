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
#ifndef AWS_S3_S3CALLBACKWRAPPER_H
#define AWS_S3_S3CALLBACKWRAPPER_H

#include <string.h>

#include <libxml/parser.h>

#include "s3/s3response.h"

namespace aws
{
  namespace s3
  {
    class S3Handler;

    class S3CallBackWrapper
    {
    public:
      S3CallBackWrapper()
        : theParserCreated(false)
      {
        memset ( &theSAXHandler, 0, sizeof ( theSAXHandler ) );
        theSAXHandler.initialized    = XML_SAX2_MAGIC;
      }
      
      ~S3CallBackWrapper()
      {
      }

      void
      createParser()
      {
        theParserCtxt = xmlCreatePushParserCtxt ( &theSAXHandler, this, NULL, 0, 0 );
        theParserCreated = true;
      }

      void
      destroyParser()
      {
        if (theParserCreated)
          xmlFreeParserCtxt ( theParserCtxt );        
      }

      bool                    theParserCreated;
      aws::s3::S3Response*    theResponse;
      aws::s3::S3Handler*     theHandler;
      xmlSAXHandler           theSAXHandler;
      xmlParserCtxtPtr        theParserCtxt;
    };

} }

#endif
