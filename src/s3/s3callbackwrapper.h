#ifndef AWS_S3_S3CALLBACKWRAPPER_H
#define AWS_S3_S3CALLBACKWRAPPER_H

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
