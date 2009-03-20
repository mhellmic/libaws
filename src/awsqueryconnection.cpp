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

#include "awsqueryconnection.h"

#include "awsquerycallback.h"

#include <openssl/hmac.h>
#include <curl/curl.h>
#include <sstream>
#include "awsqueryresponse.h"
#include <cassert>
#include <iostream>


namespace aws {

  DEFINE_LOGGER ( aws::AWSQueryConnection );

  std::string AWSQueryConnection::QUERY_DATE_FORMAT("%FT%H:%M:%SZ");

  AWSQueryConnection::AWSQueryConnection ( const std::string &aAccessKeyId,
      const std::string &aSecretAccessKey,
      const std::string& aCustomHost,
      const std::string& aVersion,
      int aPort,
      bool aIsSecure) :
      AWSConnection ( aAccessKeyId,aSecretAccessKey, aCustomHost, aPort, aIsSecure ),
      theVersion ( aVersion ),
      theSList(NULL)
  {
    // always use a content-type text/plain as required by amazon
    theSList = curl_slist_append ( theSList, "Content-Type: text/plain" );
    curl_easy_setopt ( theCurl, CURLOPT_HTTPHEADER, theSList );
    curl_easy_setopt ( theCurl, CURLOPT_WRITEFUNCTION,  AWSQueryConnection::dataReceiver );
    curl_easy_setopt ( theCurl, CURLOPT_ERRORBUFFER, theCurlErrorBuffer );
    curl_easy_setopt ( theCurl, CURLOPT_HTTPGET, 1);
  };

  AWSQueryConnection::~AWSQueryConnection()
  {
    curl_slist_free_all ( theSList );
  }

  void
  AWSQueryConnection::setCommonParamaters ( ParameterMap* aParameterMap, const std::string& aAction ) {
    aParameterMap->insert ( ParameterPair ( "AWSAccessKeyId", theAccessKeyId ) );
    aParameterMap->insert ( ParameterPair ( "Version", theVersion ) );
    aParameterMap->insert ( ParameterPair ( "SignatureVersion", "1" ) );
    aParameterMap->insert ( ParameterPair ( "Timestamp", getQueryTimestamp() ) );
    aParameterMap->insert ( ParameterPair ( "Action", aAction ) );
  }

  void
  AWSQueryConnection::makeQueryRequestOnResource ( const std::string &aResource,
                                         const std::string &action,
                                          ParameterMap* aParameterMap,
                                          QueryCallBack* aCallBack )
  {
    std::stringstream lUrlStream;
    lUrlStream << ( theIsSecure ? "https://": "http://" ) << theHost;
    if (thePort > 0) {
    	lUrlStream << ":" << thePort;
    }
    lUrlStream << "/" << aResource;
    std::string lUrl = lUrlStream.str();
    return makeQueryRequest(lUrl, action, aParameterMap, aCallBack);
  }

  void
  AWSQueryConnection::makeQueryRequest( const std::string &action,
                                        ParameterMap* aParameterMap,
                                        QueryCallBack* aCallBack )
      {
        std::stringstream lUrlStream;
        lUrlStream << ( theIsSecure ? "https://": "http://" ) << theHost;
        if (thePort > 0) {
        	lUrlStream << ":" << thePort;
        }
        std::string lUrl = lUrlStream.str();
        return makeQueryRequest(lUrl, action, aParameterMap, aCallBack);
      }

  void
  AWSQueryConnection::makeQueryRequest ( const std::string& aURL,
                                         const std::string &action,
                                         ParameterMap* aParameterMap,
                                         QueryCallBack* aCallBack )
  {
    setCommonParamaters(aParameterMap, action);

    aCallBack->theSAXHandler.startElementNs = &QueryCallBack::SAX_StartElementNs;
    aCallBack->theSAXHandler.characters     = &QueryCallBack::SAX_CharactersSAXFunc;
    aCallBack->theSAXHandler.endElementNs   = &QueryCallBack::SAX_EndElementNs;

    aCallBack->createParser();

    std::stringstream lStringToSign;
    std::stringstream lUrl;

    // begin with the url
    lUrl << aURL;

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

    //std::cout << "String to sign "  << lStringToSign.str() << std::endl;



    {
      unsigned int  lEncryptedResultSize;
      unsigned char lEncryptedResult[1024];

      // compute signature
      HMAC ( EVP_sha1(), theSecretAccessKey.c_str(),  theSecretAccessKey.size(),
             ( const unsigned char* ) lStringToSign.str().c_str(), lStringToSign.str().size(),
               lEncryptedResult, &lEncryptedResultSize );

      // append the url and base64 encoded signature
      long lBase64EncodedStringLength;
      lUrl << "&Signature=" <<
          urlencode ( base64Encode ( lEncryptedResult, lEncryptedResultSize,
                                 lBase64EncodedStringLength ) );
    }

    // necessary, in order to keep the string until the end of the function
    // can possibly be removed with a newer curl version
    // because it will always copy
    std::string lUrlString = lUrl.str();

    LOG_INFO("Send request:" << lUrlString);
    
    
    
    //std::cout << lUrlString << std::endl;
    // set the request url
    curl_easy_setopt ( theCurl, CURLOPT_URL, lUrlString.c_str() );

    // set the request method (i.e. get, post) and the according callback functions
    //setRequestMethod ( aActionType );

    // set the data object received in the callback function
    curl_easy_setopt ( theCurl, CURLOPT_WRITEDATA, ( void* ) ( aCallBack ) );

    // set a callback for retrieving all http header information
//    curl_easy_setopt ( theCurl, CURLOPT_HEADERFUNCTION, AWSQueryConnection::getHeaderData );

    //curl_easy_setopt ( theCurl, CURLOPT_VERBOSE, 1 );


    if ( ++theNumberOfRequests >= MAX_REQUESTS )
    {
      curl_easy_setopt ( theCurl, CURLOPT_FRESH_CONNECT, "TRUE" );
      theNumberOfRequests = 0;
    }

    // finally, execute the request
    CURLcode lCurlCode = curl_easy_perform ( theCurl );
    curl_easy_setopt ( theCurl, CURLOPT_FRESH_CONNECT, "FALSE" );

    //If the error code is !=0 and the handler is marked as succefully there was nothing parsed
    //so we should set the error code from the http reques
    if ( lCurlCode != 0 )
    {
      std::stringstream lTmp;
      lTmp << theCurlErrorBuffer;
      QueryErrorResponse lQER = QueryErrorResponse(lTmp.str(), lTmp.str(), "", lUrlString);
      aCallBack->theIsSuccessful = false;
      aCallBack->theQueryErrorResponse = lQER;
    } else if(aCallBack->theIsSuccessful){ //only if we haven't catched an error before, we overwrite the error with an HTTP one
    	// check HTTP response code
    	long lResponseCode = 0;
    	curl_easy_getinfo( theCurl, CURLINFO_RESPONSE_CODE, &lResponseCode );
    	if (lResponseCode >= 300) { // http response codes >= 300 are errors
    		// tested the normal case, the response was lResponseCode = 200
        std::stringstream lTmp;
        lTmp << "Errorneous HTTP status code " << lResponseCode;
        QueryErrorResponse lQER = QueryErrorResponse(lTmp.str(), lTmp.str(), "", lUrlString);
        aCallBack->theIsSuccessful = false;
        aCallBack->theQueryErrorResponse = lQER;
    	}
    }

    // signal the parse that this is the end
    xmlParseChunk ( aCallBack->theParserCtxt, 0, 0, 1 );
    
    double lDownloadSize;
    curl_easy_getinfo( theCurl, CURLINFO_SIZE_DOWNLOAD, &lDownloadSize);
    aCallBack->theInTransfer = lUrlString.size();
    aCallBack->theOutTransfer = lDownloadSize;
    aCallBack->destroyParser();
    
  }

  std::string
  AWSQueryConnection::getQueryTimestamp()
  {
    time_t lRawTime; tm*    lPtm;
    time ( &lRawTime );
    lPtm = gmtime ( &lRawTime );
    char   theDateString[31];

#ifndef NDEBUG
    size_t lTest = strftime ( theDateString, 31, QUERY_DATE_FORMAT.c_str(), lPtm );
    assert ( lTest < 31 );
#else
    strftime ( theDateString, 31, QUERY_DATE_FORMAT.c_str(), lPtm );
#endif
    return std::string ( theDateString );
  }

  size_t
  AWSQueryConnection::dataReceiver ( void *ptr, size_t size, size_t nmemb, void *data )
  {
    QueryCallBack* lQueryCallBack = static_cast<QueryCallBack*> ( data );
    char* lChars = static_cast<char*> ( ptr );

    //std::cout << lChars << std::endl;

    // this guarantees to read the input in chunks as they come in
    // by libxml; we always read as much as is in the buffer
    // because we stream internally.
    xmlParseChunk ( lQueryCallBack->theParserCtxt, lChars, size * nmemb, 0 );

    return size * nmemb;
  }
  
  void AWSQueryConnection::setCommons(QueryCallBack& aHandler, QueryResponse* aResponse){
    aResponse->theOutTransfer = aHandler.theOutTransfer;
    aResponse->theInTransfer = aHandler.theInTransfer;
  }


}//Namespace
