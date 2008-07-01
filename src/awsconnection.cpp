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
#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/bio.h>

#include <fstream>
#include <istream>
#include <map>
#include <sstream>

#include "awsconnection.h"

namespace aws { 

std::string AWSConnection::METADATA_PREFIX 		      = "x-amz-meta-";
std::string AWSConnection::AMAZON_HEADER_PREFIX 	  = "x-amz-";
std::string AWSConnection::ALTERNATIVE_DATE_HEADER  = "x-amz-date";
    
uint8_t AWSConnection::MAX_REQUESTS = 30;    

AWSConnection::AWSConnection(const std::string& aAccessKeyId, 
                             const std::string& aSecretAccessKey, 
                             const std::string& aHost)
    : theAccessKeyId(aAccessKeyId),
      theSecretAccessKey(aSecretAccessKey),
	    theHost(aHost),
      theCurlErrorBuffer(0),
      theIsSecure(false),
      theNumberOfRequests(0),
      theCurl(0),
      theBio(0),
      theB64(0)
{
  // Initialize SHA1 encryption
  HMAC_CTX_init(&theHctx);
  HMAC_Init(&theHctx, theSecretAccessKey.c_str(), theSecretAccessKey.size(), EVP_sha1());
   
  // initialization for base64 encoding stuff
  // done globally in order to cache all the stuff
  // however, the connection is not thread safe anymore
  theBio = BIO_new(BIO_s_mem());
  theB64 = BIO_new(BIO_f_base64());
  BIO_set_flags(theB64, BIO_FLAGS_BASE64_NO_NL);
  theBio = BIO_push(theB64, theBio);
  
  // curl initialization (check on every call if everything went ok
  curl_version_info_data* lVersionInfo = curl_version_info(CURLVERSION_NOW);
  int lFeatures = lVersionInfo->features;
  theIsSecure = lFeatures & CURL_VERSION_SSL;
  thePort = theIsSecure?443:80;
  
  theCurlErrorBuffer = new char[CURLOPT_ERRORBUFFER];

  theCurl = curl_easy_init();
}

AWSConnection::~AWSConnection()
{    
  curl_easy_cleanup(theCurl);

  delete[] theCurlErrorBuffer; theCurlErrorBuffer = 0;

  BIO_free_all(theBio);    

  HMAC_CTX_cleanup(&theHctx);
}

std::string
AWSConnection::base64Encode(const char* aContent, size_t aContentSize, 
                            long& aBase64EncodedStringLength)
{
  char* lEncodedString;

  BIO_write(theBio, aContent, aContentSize);
  BIO_flush(theBio);
  aBase64EncodedStringLength = BIO_get_mem_data(theBio, &lEncodedString);

  // ensures null termination
  std::stringstream lTmp;
  lTmp.write(lEncodedString, aBase64EncodedStringLength);
  
  BIO_reset(theBio);
  BIO_reset(theB64);
  
  return lTmp.str(); // copy
}

std::string
AWSConnection::base64Encode(const unsigned char* aContent, size_t aContentSize, 
                            long& aBase64EncodedStringLength)
{
  char* lEncodedString;
  
  BIO_write(theBio, aContent, aContentSize);
  BIO_flush(theBio);
  aBase64EncodedStringLength = BIO_get_mem_data(theBio, &lEncodedString);

  // ensures null termination
  std::stringstream lTmp;
  lTmp.write(lEncodedString, aBase64EncodedStringLength);
  
  BIO_reset(theBio);
  BIO_reset(theB64);
  
  return lTmp.str(); // copy
}

std::string
aws::AWSConnection::urlencode(const std::string &aStringToEncode)
{
  char* lUrlEncodedValue = 
      curl_escape(const_cast<char*>(aStringToEncode.c_str()), 
                                    aStringToEncode.size());
  // copy the string    
  std::string lRes(lUrlEncodedValue);
  
  curl_free(lUrlEncodedValue);
  
  return lRes; // unfortunately, copy again
}

} /* namespace aws */
