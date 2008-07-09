#include "common.h"

#include "awsqueryconnection.h"

#include "awsquerycallback.h"

#include <openssl/hmac.h>
#include <curl/curl.h>
#include <sstream>



namespace aws {

  std::string AWSQueryConnection::QUERY_DATE_FORMAT("%FT%H:%M:%SZ");

  AWSQueryConnection::AWSQueryConnection ( const std::string &aAccessKeyId,
      const std::string &aSecretAccessKey,
      const std::string& aHost,
      const std::string& aVersion ) :
      theVersion ( aVersion ), AWSConnection ( aAccessKeyId,aSecretAccessKey, aHost )  {
    // always use a content-type text/plain as required by amazon
    theSList = curl_slist_append ( theSList, "Content-Type: text/plain" );
    curl_easy_setopt ( theCurl, CURLOPT_HTTPHEADER, theSList );
    curl_easy_setopt ( theCurl, CURLOPT_WRITEFUNCTION,  AWSQueryConnection::dataReceiver );
    curl_easy_setopt ( theCurl, CURLOPT_ERRORBUFFER, theCurlErrorBuffer );
    curl_easy_setopt ( theCurl, CURLOPT_HTTPGET, 1); 

    std::stringstream lUrl;
    lUrl << ( theIsSecure ? "https://": "http://" ) << aHost << ":" << thePort;
    theUrl = lUrl.str();
  };

  AWSQueryConnection::~AWSQueryConnection()
  {
    curl_slist_free_all ( theSList );
  }

  void AWSQueryConnection::setCommonParamaters ( ParameterMap* aParameterMap, const std::string& aAction ) {
    aParameterMap->insert ( ParameterPair ( "AWSAccessKeyId", theAccessKeyId ) );
    aParameterMap->insert ( ParameterPair ( "Version", theVersion ) );
    aParameterMap->insert ( ParameterPair ( "Timestamp", getQueryTimestamp() ) );
    aParameterMap->insert ( ParameterPair ( "Action", aAction ) );
  }

  void
  AWSQueryConnection::makeQueryRequest ( const std::string &action,  ParameterMap* aParameterMap, QueryCallBack* aCallBack )
  {
    setCommonParamaters(aParameterMap, action);
    
    std::stringstream lStringToSign;
    std::stringstream lUrl;

    // begin with the url
    lUrl << theUrl;

    // build query url and the string to sign
    bool lFirst = true;
    for ( ParameterMapIter lIter = aParameterMap->begin();
          lIter != aParameterMap->end(); ++lIter )
    {
      lUrl << ( lFirst?lFirst=false,"/?":"&" );

      // url encode each value
      lUrl << ( *lIter ).first << "=" << urlencode ( ( *lIter ).second );

      // concatenate parameter name and value for the string to sign
      lStringToSign << ( *lIter ).first << ( *lIter ).second;
    }

    {
      unsigned int  theEncryptedResultSize;
      unsigned char theEncryptedResult[1024];
      char*         theBase64EncodedString;

      // compute signature
      HMAC ( EVP_sha1(), theSecretAccessKey.c_str(),  theSecretAccessKey.size(),
             ( const unsigned char* ) lStringToSign.str().c_str(), lStringToSign.str().size(),
             theEncryptedResult, &theEncryptedResultSize );

      // append the url and base64 encoded signature
      long lBase64EncodedStringLength;
      lUrl << "&Signature=" <<
      urlencode ( base64Encode ( theEncryptedResult, theEncryptedResultSize,
                                 lBase64EncodedStringLength ) );
    }

    // necessary, in order to keep the string until the end of the function
    // can possibly be removed with a newer curl version
    // because it will always copy
    std::string lUrlString = lUrl.str();

    //std::cout << "Send request:" << lUrlString << std::endl;

    // set the request url
    curl_easy_setopt ( theCurl, CURLOPT_URL, lUrlString.c_str() );

    // set the request method (i.e. get, post) and the according callback functions
    //setRequestMethod ( aActionType );

    // set the data object received in the callback function
    curl_easy_setopt ( theCurl, CURLOPT_WRITEDATA, ( void* ) ( aCallBack ) );

    // set a callback for retrieving all http header information
//    curl_easy_setopt ( theCurl, CURLOPT_HEADERFUNCTION, AWSQueryConnection::getHeaderData );

#if 0
    curl_easy_setopt ( theCurl, CURLOPT_VERBOSE, 1 );
#endif


    if ( ++theNumberOfRequests >= MAX_REQUESTS )
    {
      curl_easy_setopt ( theCurl, CURLOPT_FRESH_CONNECT, "TRUE" );
      theNumberOfRequests = 0;
    }

    // finally, execute the request
    CURLcode lCurlCode = curl_easy_perform ( theCurl );
    curl_easy_setopt ( theCurl, CURLOPT_FRESH_CONNECT, "FALSE" );

    if ( lCurlCode != 0 )
    {
      SQSResponseError::Error lError;
      lError.theErrorCode = SQSResponseError::CurlError;
      lError.theErrorMessage = std::string ( theCurlErrorBuffer );
      lResponse->theSQSResponseError.theErrors.push_back ( lError );
      lResponse->theIsSuccessful = false;
    }

    // signal the parse that this is the end
    xmlParseChunk ( aCallBackWrapper->theParserCtxt, 0, 0, 1 );

  }

  std::string
  AWSQueryConnection::getQueryTimestamp()
  {
    time_t lRawTime; tm*    lPtm;
    time ( &lRawTime );
    lPtm = gmtime ( &lRawTime );

    size_t lTest = strftime ( theDateString, 31, QUERY_DATE_FORMAT.c_str(), lPtm );
    assert ( lTest < 31 );
    return std::string ( theDateString );
  }

  size_t
  AWSQueryConnection::dataReceiver ( void *ptr, size_t size, size_t nmemb, void *data )
  {
    QueryCallBack* lQueryCallBack = static_cast<QueryCallBack*> ( data );

    char* lChars = static_cast<char*> ( ptr );

    // this guarantees to read the input in chunks as they come in
    // by libxml; we always read as much as is in the buffer
    // because we stream internally.
    xmlParseChunk ( lQueryCallBack, lChars, size * nmemb, 0 );

    return size * nmemb;
  }


}//Namespace
