#include "aws/common.h"

#include <openssl/hmac.h>


#include "sqshandler.h"
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
    std::string localname ( ( const char* ) value, len );
    startElement ( localname, nb_attributes, attributes );
  }

  void SimpleQueryCallBack::charactersSAXFunc ( const xmlChar * value,
      int len ) {
    std::string value ( ( const char* ) value, len );
    characters ( value );

  }

  void SimpleQueryCallBack::endElementNs ( const xmlChar * localname,
      const xmlChar * prefix,
      const xmlChar * URI ) {
    std::string localname ( ( const char* ) value, len );
    endElement ( localname );
  }

  static void QueryCallBack::SAX_StartElementNs ( void * ctx,
      const xmlChar * localname,
      const xmlChar * prefix,
      const xmlChar * URI,
      int nb_namespaces,
      const xmlChar ** namespaces,
      int nb_attributes,
      int nb_defaulted,
      const xmlChar ** attributes ) {
    QueryCallBack* lCallBack  = static_cast<QueryCallBack*> ( ctx );
    lCallBack->endElementNs ( localname, prefix, URI, nb_namespaces, namespaces, nb_attributes, nb_defaulted, attributes );
  }

  static void QueryCallBack::SAX_CharactersSAXFunc ( void * ctx,
      const xmlChar * value,
      int len ) {
    QueryCallBack* lCallBack  = static_cast<QueryCallBack*> ( ctx );
    lCallBack->endElementNs ( value, len );
  }

  static void QueryCallBack::SAX_EndElementNs ( void * ctx,
      const xmlChar * localname,
      const xmlChar * prefix,
      const xmlChar * URI ) {
    QueryCallBack* lCallBack  = static_cast<QueryCallBack*> ( ctx );
    lCallBack->endElementNs ( localname, prefix, URI );
  }

}//namespace
