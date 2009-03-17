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
#ifndef AWS_AWSCONNECTION_H
#define AWS_AWSCONNECTION_H

#include <openssl/hmac.h>
#include "common.h"

struct bio_st;
typedef struct bio_st BIO;
typedef void CURLM;
typedef void CURL;

class OpenSSLData;

namespace aws {

class AWSConnection {

public:
  static
  std::string urlEncode(const std::string& aContent);

  static
  std::string base64Encode(const char* aContent, size_t aContentSize,
                           long &aBase64EncodedStringLength);

  static
  std::string base64Encode(const unsigned char* aContent, size_t aContentSize,
                           long &aBase64EncodedStringLength);

  static
  const char* base64Decode(const char* a64Content, size_t a64ContentSize,
													 size_t &aDecodedStringLength);

protected:
    friend class RequestHeaderMap;
    static std::string AMAZON_HEADER_PREFIX;
    static std::string ALTERNATIVE_DATE_HEADER;
    static uint8_t  MAX_REQUESTS;

    std::string theAccessKeyId;
    std::string theSecretAccessKey;
	  std::string	theHost;
    char*       theCurlErrorBuffer;

    bool        theIsSecure;
    uint8_t     theNumberOfRequests; // used for resetting the connection once in a while
    int         thePort;
    CURL*       theCurl; // maybe a pool later
    HMAC_CTX    theHctx;

    // moved these vars into static function
    // BIO*        theBio;
    // BIO*        theB64;


    AWSConnection(const std::string& aAccessKeyId, const std::string& aSecretAccessKey,
                  const std::string& aHost, int aPort, bool aIsSecure);


    static std::string urlencode(const std::string&);

public:
    virtual ~AWSConnection();

};

} /* namespace aws */
#endif /* !AWS_AWSCONNECTION_H */
