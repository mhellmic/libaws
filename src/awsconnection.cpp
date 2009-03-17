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
#include "libaws/config.h"
#include <curl/curl.h>
#include <openssl/evp.h>
#include <openssl/bio.h>

#include <fstream>
#include <istream>
#include <map>
#include <sstream>

#include "awsconnection.h"

namespace aws {

std::string AWSConnection::AMAZON_HEADER_PREFIX 	  = "x-amz-";
std::string AWSConnection::ALTERNATIVE_DATE_HEADER  = "x-amz-date";

uint8_t AWSConnection::MAX_REQUESTS = 30;

AWSConnection::AWSConnection(const std::string& aAccessKeyId,
                             const std::string& aSecretAccessKey,
                             const std::string& aHost,
                             int aPort,
                             bool aIsSecure)
    : theAccessKeyId(aAccessKeyId),
      theSecretAccessKey(aSecretAccessKey),
	    theHost(aHost),
      theCurlErrorBuffer(0),
      theIsSecure(false),
      theNumberOfRequests(0),
      thePort(aPort),
      theCurl(0)
{
  // Initialize SHA1 encryption
  HMAC_CTX_init(&theHctx);
  HMAC_Init(&theHctx, theSecretAccessKey.c_str(), theSecretAccessKey.size(), EVP_sha1());

  // curl initialization (check on every call if everything went ok
#ifdef WITH_SSL
  curl_version_info_data* lVersionInfo = curl_version_info(CURLVERSION_NOW);
  int lFeatures = lVersionInfo->features;
  theIsSecure = aIsSecure && (lFeatures & CURL_VERSION_SSL);
  if (theIsSecure) {
  	thePort = 443;
  }
#endif
  theCurlErrorBuffer = new char[CURLOPT_ERRORBUFFER];

  theCurl = curl_easy_init();

}

AWSConnection::~AWSConnection()
{
  curl_easy_cleanup(theCurl);

  delete[] theCurlErrorBuffer; theCurlErrorBuffer = 0;

  HMAC_CTX_cleanup(&theHctx);
}

std::string
AWSConnection::urlEncode(const std::string& aContent)
{
  std::string encoded;
  unsigned char c;
  unsigned char low, high;

  for (size_t i = 0; i < aContent.size(); i++) {
    c = aContent[i];
    if (isalnum(c))
       encoded += c;
    else {
       high = c / 16;
       low = c % 16;
       encoded += '%';
       encoded += (high < 10 ? '0' + high : 'A' + high - 10);
       encoded += (low < 10 ? '0' + low : 'A' + low - 10);
    }
  }
  return encoded;
}

std::string
AWSConnection::base64Encode(const char* aContent, size_t aContentSize,
                            long& aBase64EncodedStringLength)
{
  char* lEncodedString;

  // initialization for base64 encoding stuff
  BIO* lBio = BIO_new(BIO_s_mem());
  BIO* lB64 = BIO_new(BIO_f_base64());
  BIO_set_flags(lB64, BIO_FLAGS_BASE64_NO_NL);
  lBio = BIO_push(lB64, lBio);

  BIO_write(lBio, aContent, aContentSize);
  BIO_flush(lBio);
  aBase64EncodedStringLength = BIO_get_mem_data(lBio, &lEncodedString);

  // ensures null termination
  std::stringstream lTmp;
  lTmp.write(lEncodedString, aBase64EncodedStringLength);

  BIO_free_all(lBio);

  return lTmp.str(); // copy
}

std::string
AWSConnection::base64Encode(const unsigned char* aContent, size_t aContentSize,
                            long& aBase64EncodedStringLength)
{
  char* lEncodedString;

  // initialization for base64 encoding stuff
  BIO* lBio = BIO_new(BIO_s_mem());
  BIO* lB64 = BIO_new(BIO_f_base64());
  BIO_set_flags(lB64, BIO_FLAGS_BASE64_NO_NL);
  lBio = BIO_push(lB64, lBio);

  BIO_write(lBio, aContent, aContentSize);
  BIO_flush(lBio);
  aBase64EncodedStringLength = BIO_get_mem_data(lBio, &lEncodedString);

  // ensures null termination
  std::stringstream lTmp;
  lTmp.write(lEncodedString, aBase64EncodedStringLength);

  BIO_free_all(lBio);
  return lTmp.str(); // copy
}

const char*
AWSConnection::base64Decode(const char* a64Content, size_t a64ContentSize, size_t &aDecodedStringLength) {

  // initialization for base64 decoding stuff
  BIO* lBio = BIO_new_mem_buf((char*) a64Content, a64ContentSize);
	BIO* lB64 = BIO_new(BIO_f_base64());
	BIO_set_flags(lB64, BIO_FLAGS_BASE64_NO_NL);
	lBio = BIO_push(lB64, lBio);

	// decode into an NSMutableData
	char* lStr = new char[8192];
	aDecodedStringLength = BIO_read(lBio, lStr, 8192);

	// clean up and go home
	BIO_free_all(lBio);
  return lStr;
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
