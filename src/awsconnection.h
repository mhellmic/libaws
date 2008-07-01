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

protected:   
    friend class RequestHeaderMap;
    static std::string METADATA_PREFIX;
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
    BIO*        theBio;
    BIO*        theB64;
    
    
    AWSConnection(const std::string& aAccessKeyId, const std::string& aSecretAccessKey, 
                  const std::string& aHost);
    
  
    std::string base64Encode(const char* aContent, size_t aContentSize,
                             long &aBase64EncodedStringLength);

    std::string base64Encode(const unsigned char* aContent, size_t aContentSize,
                             long &aBase64EncodedStringLength);

    
    static std::string urlencode(const std::string&);

public:
    virtual ~AWSConnection();

};

} /* namespace aws */
#endif /* !AWS_AWSCONNECTION_H */
