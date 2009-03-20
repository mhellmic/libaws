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

#include <memory>
#include <curl/curl.h>
#include <openssl/hmac.h>
#include <cassert>

#include "requestheadermap.h"
#include "response.h"
#include "canonizer.h"
#include "callingformat.h"
#include "util.h"
#include "curlstreambuf.h"


#include "s3/s3connection.h"
#include "s3/s3object.h"
#include "s3/s3handler.h"
#include "s3/s3response.h"
#include "s3/s3callbackwrapper.h"

using namespace aws;

namespace aws { namespace s3 {

#define REQUEST_PROLOG(REQUESTNAME)                                                   \
  REQUESTNAME ## Handler  lHandler;                                                    \
  S3CallBackWrapper       lWrapper;                                                   \
                                                                                      \
  lWrapper.theResponse  = lRes.get();                                                 \
  lWrapper.theHandler   = &lHandler;                                                  \
                                                                                      \
  lWrapper.theSAXHandler.startElementNs = &REQUESTNAME ## Handler::startElementNs;    \
  lWrapper.theSAXHandler.characters     = &REQUESTNAME ## Handler::charactersSAXFunc; \
  lWrapper.theSAXHandler.endElementNs   = &REQUESTNAME ## Handler::endElementNs;      \
                                                                                      \
  lWrapper.createParser();                                                            \
                                                                                      \
  try {

#define REQUEST_EPILOG(REQUESTNAME)                                \
  } catch (AWSConnectionException& ce) {                           \
    lWrapper.destroyParser();                                      \
    throw ce;                                                      \
  } catch (AWSException& e) {                                      \
    lWrapper.destroyParser();                                      \
    throw e;                                                       \
  }                                                                \
  lWrapper.destroyParser();                                        \
                                                                   \
  if ( ! lRes->isSuccessful() )                                    \
    throw REQUESTNAME ## Exception( lRes->theS3ResponseError );



std::string S3Connection::DEFAULT_HOST = "s3.amazonaws.com";

S3Connection::S3Connection(const std::string& aAccessKeyId, const std::string& aSecretAccessKey,
                           const std::string& aCustomHost)
  : AWSConnection(aAccessKeyId, aSecretAccessKey, aCustomHost.size()==0?DEFAULT_HOST:aCustomHost, -1, true),
    theEncryptedResultSize(0),
    theBase64EncodedString(0)
{
  // set callbacks for retrieving all http header information
  curl_easy_setopt(theCurl, CURLOPT_HEADERFUNCTION, S3Connection::getHeaderData);

  curl_easy_setopt(theCurl, CURLOPT_ERRORBUFFER, theCurlErrorBuffer);

  // we enforce http 1.0 here in order to not use transfer-encoding: chunked
  // amazon doesn't understand that
  curl_easy_setopt(theCurl, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);

}

S3Connection::~S3Connection() {  }

// Bucket handling functions
CreateBucketResponse*
S3Connection::createBucket(const std::string& aBucketName)
{
  std::auto_ptr<CreateBucketResponse> lRes(new CreateBucketResponse(aBucketName));

  REQUEST_PROLOG(CreateBucket);

  makeRequest(aBucketName, CREATE_BUCKET, &lWrapper, 0, 0);

  REQUEST_EPILOG(CreateBucket);

  return lRes.release();
}

ListAllBucketsResponse*
S3Connection::listAllBuckets()
{
  std::auto_ptr<ListAllBucketsResponse> lRes(new ListAllBucketsResponse());

  REQUEST_PROLOG(ListAllBuckets);

  makeRequest("", LIST_ALL_BUCKETS, &lWrapper, 0, 0);

  REQUEST_EPILOG(ListAllBuckets);

  return lRes.release();
}

ListBucketResponse*
S3Connection::listBucket(const std::string& aBucketName, const std::string& aPrefix,
                         const std::string& aMarker, int aMaxKeys)
{
  std::auto_ptr<ListBucketResponse> lRes(new ListBucketResponse(aBucketName, aPrefix,
                                                                aMarker, aMaxKeys));

  ListBucketHandler     lHandler;

  S3CallBackWrapper       lWrapper;
  lWrapper.theResponse  = lRes.get();
  lWrapper.theHandler   = &lHandler;

  lWrapper.theSAXHandler.startElementNs = &ListBucketHandler::startElementNs;
  lWrapper.theSAXHandler.characters     = &ListBucketHandler::charactersSAXFunc;
  lWrapper.theSAXHandler.endElementNs   = &ListBucketHandler::endElementNs;

  PathArgs_t lPathArgsMap;

  char* lEscapedPrefixChar = curl_escape(aPrefix.c_str(), aPrefix.size());
  char* lEscapedMarkerChar = curl_escape(aMarker.c_str(), aMarker.size());

  std::string lEscapedPrefix(lEscapedPrefixChar);
  std::string lEscapedMarker(lEscapedMarkerChar);

  if (lEscapedPrefix.size() != 0)
      lPathArgsMap.insert(stringpair_t("prefix", lEscapedPrefix));

  if (lEscapedMarker.size() != 0)
      lPathArgsMap.insert(stringpair_t("marker", lEscapedMarker));

  if (aMaxKeys != -1) {
      std::stringstream s;
      s << aMaxKeys;
      lPathArgsMap.insert(stringpair_t("max-keys", s.str()));
  }

  lWrapper.createParser();

  try {
    makeRequest(aBucketName, LIST_BUCKET, &lWrapper, &lPathArgsMap, 0);
  } catch (AWSException& e) {
    lWrapper.destroyParser();
    curl_free(lEscapedPrefixChar);
    curl_free(lEscapedMarkerChar);
    throw e;
  }
  lWrapper.destroyParser();
  curl_free(lEscapedPrefixChar);
  curl_free(lEscapedMarkerChar);

  if ( ! lRes->isSuccessful() )
    throw ListBucketException( lRes->theS3ResponseError );

  return lRes.release();
}

ListBucketResponse*
S3Connection::listBucket(const std::string& aBucketName, const std::string& aPrefix,
                         const std::string& aMarker, const std::string& aDelimiter, int aMaxKeys)
{
  std::auto_ptr<ListBucketResponse> lRes(new ListBucketResponse(aBucketName, aPrefix,
                                                                aMarker, aMaxKeys));

  ListBucketHandler     lHandler;

  S3CallBackWrapper       lWrapper;
  lWrapper.theResponse  = lRes.get();
  lWrapper.theHandler   = &lHandler;

  lWrapper.theSAXHandler.startElementNs = &ListBucketHandler::startElementNs;
  lWrapper.theSAXHandler.characters     = &ListBucketHandler::charactersSAXFunc;
  lWrapper.theSAXHandler.endElementNs   = &ListBucketHandler::endElementNs;

  PathArgs_t lPathArgsMap;

  char* lEscapedPrefixChar = curl_escape(aPrefix.c_str(), aPrefix.size());
  char* lEscapedMarkerChar = curl_escape(aMarker.c_str(), aMarker.size());
  char* lEscapedDelimiterChar = curl_escape(aDelimiter.c_str(), aDelimiter.size());

  std::string lEscapedPrefix(lEscapedPrefixChar);
  std::string lEscapedMarker(lEscapedMarkerChar);
  std::string lEscapedDelimiter(lEscapedDelimiterChar);

  if (lEscapedPrefix.size() != 0)
      lPathArgsMap.insert(stringpair_t("prefix", lEscapedPrefix));

  if (lEscapedMarker.size() != 0)
      lPathArgsMap.insert(stringpair_t("marker", lEscapedMarker));

  if (lEscapedDelimiter.size() != 0)
      lPathArgsMap.insert(stringpair_t("delimiter", lEscapedDelimiterChar));

  if (aMaxKeys != -1) {
      std::stringstream s;
      s << aMaxKeys;
      lPathArgsMap.insert(stringpair_t("max-keys", s.str()));
  }

  lWrapper.createParser();

  try {
    makeRequest(aBucketName, LIST_BUCKET, &lWrapper, &lPathArgsMap, 0);
  } catch (AWSException& e) {
    lWrapper.destroyParser();
    curl_free(lEscapedPrefixChar);
    curl_free(lEscapedMarkerChar);
    curl_free(lEscapedDelimiterChar);
    throw e;
  }
  lWrapper.destroyParser();
  curl_free(lEscapedPrefixChar);
  curl_free(lEscapedMarkerChar);
  curl_free(lEscapedDelimiterChar);

  if ( ! lRes->isSuccessful() )
    throw ListBucketException( lRes->theS3ResponseError );

  return lRes.release();
}

DeleteBucketResponse*
S3Connection::deleteBucket(const std::string& aBucketName, RequestHeaderMap* aHeaderMap)
{
  std::auto_ptr<DeleteBucketResponse> lRes(new DeleteBucketResponse(aBucketName));

  REQUEST_PROLOG(DeleteBucket);

  makeRequest(aBucketName, DELETE_BUCKET, &lWrapper, 0, aHeaderMap);

  REQUEST_EPILOG(DeleteBucket);

  if ( ! lRes->isSuccessful() )
    throw DeleteBucketException( lRes->theS3ResponseError );

  return lRes.release();
}

PutResponse*
S3Connection::put(const std::string& aBucketName,
                  const std::string& aKey,
                  std::istream& aObject,
                  const std::string& aContentType,
                  const std::map<std::string, std::string>* aMetaDataMap,
                  long aSize)
{
  std::auto_ptr<PutResponse> lRes(new PutResponse(aBucketName));

  PutHandler             lHandler;

  S3CallBackWrapper       lWrapper;
  lWrapper.theResponse  = lRes.get();
  lWrapper.theHandler   = &lHandler;

  lWrapper.theSAXHandler.startElementNs = &PutHandler::startElementNs;
  lWrapper.theSAXHandler.characters     = &PutHandler::charactersSAXFunc;
  lWrapper.theSAXHandler.endElementNs   = &PutHandler::endElementNs;

  char* lEscapedKeyChar = curl_escape(aKey.c_str(), aKey.size());
  std::string lEscapedKey(lEscapedKeyChar);
  curl_free(lEscapedKeyChar);

  lWrapper.createParser();

  try {
    S3Object lObject;
    lObject.theIstream = &aObject;
    lObject.theContentType = aContentType;

    if (aSize == -1) {
      // determine object size
      aObject.seekg(0, std::ios_base::beg);
      std::istream::pos_type begin_pos = aObject.tellg();
      aObject.seekg(0, std::ios_base::end);
      lObject.theContentLength = aObject.tellg() - begin_pos;
      lObject.theIstream->seekg(0, std::ios_base::beg);
    } else {
      lObject.theContentLength = aSize;
    }

    if (aMetaDataMap) {
      RequestHeaderMap lRequestHeaderMap;
      for (std::map<std::string, std::string>::const_iterator lIter = aMetaDataMap->begin();
           lIter != aMetaDataMap->end(); ++lIter) {
        lRequestHeaderMap.addHeader("x-amz-meta-" + (*lIter).first, (*lIter).second);
      }

      makeRequest(aBucketName, PUT, &lWrapper, 0, &lRequestHeaderMap, lEscapedKey, &lObject);
    } else {
      makeRequest(aBucketName, PUT, &lWrapper, 0, 0, lEscapedKey, &lObject);
    }
  } catch (AWSException& e) {
    lWrapper.destroyParser();
    throw e;
  }

  lWrapper.destroyParser();

  if ( ! lRes->isSuccessful() )
    throw PutException( lRes->theS3ResponseError );

  return lRes.release();
}

PutResponse*
S3Connection::put(const std::string& aBucketName,
                  const std::string& aKey,
                  const char* aObject,
                  const std::string& aContentType,
                  const std::map<std::string, std::string>* aMetaDataMap,
                  long aSize)
{
  std::auto_ptr<PutResponse> lRes(new PutResponse(aBucketName));

  PutHandler             lHandler;

  S3CallBackWrapper       lWrapper;
  lWrapper.theResponse  = lRes.get();
  lWrapper.theHandler   = &lHandler;

  lWrapper.theSAXHandler.startElementNs = &PutHandler::startElementNs;
  lWrapper.theSAXHandler.characters     = &PutHandler::charactersSAXFunc;
  lWrapper.theSAXHandler.endElementNs   = &PutHandler::endElementNs;

  char* lEscapedKeyChar = curl_escape(aKey.c_str(), aKey.size());
  std::string lEscapedKey(lEscapedKeyChar);
  curl_free(lEscapedKeyChar);

  lWrapper.createParser();

  try {
    S3Object lObject;
    lObject.theDataPointer = aObject;
    lObject.theContentType = aContentType;
  lObject.theContentLength = aSize;

    if (aMetaDataMap) {
      RequestHeaderMap lRequestHeaderMap;
      for (std::map<std::string, std::string>::const_iterator lIter = aMetaDataMap->begin();
           lIter != aMetaDataMap->end(); ++lIter) {
        lRequestHeaderMap.addHeader("x-amz-meta-" + (*lIter).first, (*lIter).second);
      }

      makeRequest(aBucketName, PUT, &lWrapper, 0, &lRequestHeaderMap, lEscapedKey, &lObject);
    } else {
      makeRequest(aBucketName, PUT, &lWrapper, 0, 0, lEscapedKey, &lObject);
    }
  } catch (AWSException& e) {
    lWrapper.destroyParser();
    throw e;
  }

  lWrapper.destroyParser();

  if ( ! lRes->isSuccessful() )
    throw PutException( lRes->theS3ResponseError );

  return lRes.release();
}

std::string
S3Connection::queryString(ActionType aActionType,
                   const std::string& aBucketName, const std::string& aKey,
                   time_t aExpiration)
{
  RequestHeaderMap lHeaderMap;
  std::string lStringToSign;
  std::string lSignature;
  std::string lExpireString;
  std::stringstream stream;
  long lBase64EncodedStringLength;

  stream << aExpiration;
  lExpireString = stream.str();

  lHeaderMap.addHeader("Expires", lExpireString);
  lStringToSign = Canonizer::canonicalize(aActionType, aBucketName, aKey,
                                          &lHeaderMap);
  HMAC(EVP_sha1(), theSecretAccessKey.c_str(), theSecretAccessKey.size(),
       (const unsigned char *) lStringToSign.c_str(), lStringToSign.size(),
       theEncryptedResult, &theEncryptedResultSize);
  lSignature = base64Encode(theEncryptedResult, theEncryptedResultSize,
               lBase64EncodedStringLength);
  lSignature = urlEncode(lSignature);

  stream.str("");
  stream << "http://" << aBucketName << "." << DEFAULT_HOST << "/" <<
            aKey << "?AWSAccessKeyId=" << theAccessKeyId << "&Expires=" <<
            lExpireString << "&Signature=" << lSignature;
  return stream.str();
}

GetResponse*
S3Connection::get(const std::string& aBucketName, const std::string& aKey, 
                  const std::map<std::string, std::string>* aMetaDataMap)
{
  std::auto_ptr<GetResponse> lRes(new GetResponse(aBucketName, aKey));

  GetHandler             lHandler;

  S3CallBackWrapper       lWrapper;
  lWrapper.theResponse  = lRes.get();
  lWrapper.theHandler   = &lHandler;

  lWrapper.theSAXHandler.startElementNs = &GetHandler::startElementNs;
  lWrapper.theSAXHandler.characters     = &GetHandler::charactersSAXFunc;
  lWrapper.theSAXHandler.endElementNs   = &GetHandler::endElementNs;

  char* lEscapedKeyChar = curl_escape(aKey.c_str(), aKey.size());
  std::string lEscapedKey(lEscapedKeyChar);

  lWrapper.createParser();

  try {

    if (aMetaDataMap) {
      RequestHeaderMap lRequestHeaderMap;
      for (std::map<std::string, std::string>::const_iterator lIter = aMetaDataMap->begin();
           lIter != aMetaDataMap->end(); ++lIter) {
        lRequestHeaderMap.addHeader("x-amz-meta-" + (*lIter).first, (*lIter).second);
      }

      makeRequest(aBucketName, GET, &lWrapper, 0, &lRequestHeaderMap, lEscapedKey, 0);
    } else {
      makeRequest(aBucketName, GET, &lWrapper, 0, 0, lEscapedKey, 0);
    }

  } catch (AWSException& e) {
    lWrapper.destroyParser();
    curl_free(lEscapedKeyChar);
    throw e;
  }

  lWrapper.destroyParser();
  curl_free(lEscapedKeyChar);

  if ( ! lRes->isSuccessful() )
    throw GetException( lRes->theS3ResponseError );

  return lRes.release();
}


GetResponse*
S3Connection::get(const std::string& aBucketName, const std::string& aKey,
                  const std::string& aOldEtag)
{
  std::auto_ptr<GetResponse> lRes(new GetResponse(aBucketName, aKey));

  GetHandler             lHandler;

  S3CallBackWrapper       lWrapper;
  lWrapper.theResponse  = lRes.get();
  lWrapper.theHandler   = &lHandler;

  lWrapper.theSAXHandler.startElementNs = &GetHandler::startElementNs;
  lWrapper.theSAXHandler.characters     = &GetHandler::charactersSAXFunc;
  lWrapper.theSAXHandler.endElementNs   = &GetHandler::endElementNs;

  char* lEscapedKeyChar = curl_escape(aKey.c_str(), aKey.size());
  std::string lEscapedKey(lEscapedKeyChar);

  RequestHeaderMap lRequestHeaderMap;
  lRequestHeaderMap.addHeader("If-None-Match",aOldEtag);

  lWrapper.createParser();

  try {
    makeRequest(aBucketName, GET, &lWrapper, 0, &lRequestHeaderMap, lEscapedKey, 0);
  } catch (AWSException& e) {
    lWrapper.destroyParser();
    curl_free(lEscapedKeyChar);
    throw e;
  }

  lWrapper.destroyParser();

  curl_free(lEscapedKeyChar);

  if ( ! lRes->isSuccessful() )
    throw GetException( lRes->theS3ResponseError );

  return lRes.release();
}

DeleteResponse*
S3Connection::del(const std::string& aBucketName, const std::string& aKey)
{
  std::auto_ptr<DeleteResponse> lRes(new DeleteResponse(aBucketName, aKey));

  DeleteHandler           lHandler;

  S3CallBackWrapper       lWrapper;
  lWrapper.theResponse  = lRes.get();
  lWrapper.theHandler   = &lHandler;

  lWrapper.theSAXHandler.startElementNs = &DeleteHandler::startElementNs;
  lWrapper.theSAXHandler.characters     = &DeleteHandler::charactersSAXFunc;
  lWrapper.theSAXHandler.endElementNs   = &DeleteHandler::endElementNs;

  char* lEscapedKeyChar = curl_escape(aKey.c_str(), aKey.size());
  std::string lEscapedKey(lEscapedKeyChar);

  lWrapper.createParser();

  try {
    makeRequest(aBucketName, DELETE, &lWrapper, 0, 0, lEscapedKey, 0);
  } catch (AWSException& e) {
    lWrapper.destroyParser();
    curl_free(lEscapedKeyChar);
    throw e;
  }

  lWrapper.destroyParser();

  curl_free(lEscapedKeyChar);

  if ( ! lRes->isSuccessful() )
    throw DeleteException( lRes->theS3ResponseError );

  return lRes.release();
}

DeleteAllResponse*
S3Connection::deleteAll(const std::string& aBucketName, const std::string& aPrefix)
{
  std::auto_ptr<DeleteAllResponse> lRes(new DeleteAllResponse(aBucketName, aPrefix));

  std::auto_ptr<ListBucketResponse> lListBucket;
  std::auto_ptr<DeleteResponse> lDelete;
  ListBucketResponse::Key lKey;
  std::string lMarker;
  try {
    do {
      lListBucket.reset(listBucket(aBucketName, aPrefix, lMarker, -1));
      lListBucket->open();
      while (lListBucket->next(lKey)) {
        lDelete.reset(del(aBucketName, lKey.KeyValue));
        lMarker = lKey.KeyValue;
      }
      lListBucket->close();
    } while (lListBucket->isTruncated());
  } catch (ListBucketException &e) {
    throw DeleteAllException( e.getErrorCode(), e.getErrorMessage(), e.getRequestId(), e.getHostId() );
  } catch (DeleteException &lDelExc) {
    throw DeleteAllException( lDelExc.getErrorCode(), lDelExc.getErrorMessage(), lDelExc.getRequestId(), lDelExc.getHostId() );
  }

  return lRes.release();
}

HeadResponse*
S3Connection::head(const std::string& aBucketName, const std::string& aKey)
{
  std::auto_ptr<HeadResponse> lRes(new HeadResponse(aBucketName));

  HeadHandler             lHandler;

  S3CallBackWrapper       lWrapper;
  lWrapper.theResponse  = lRes.get();
  lWrapper.theHandler   = &lHandler;

  lWrapper.theSAXHandler.startElementNs = &HeadHandler::startElementNs;
  lWrapper.theSAXHandler.characters     = &HeadHandler::charactersSAXFunc;
  lWrapper.theSAXHandler.endElementNs   = &HeadHandler::endElementNs;

  char* lEscapedKeyChar = curl_escape(aKey.c_str(), aKey.size());
  std::string lEscapedKey(lEscapedKeyChar);

  lWrapper.createParser();

  try {
    makeRequest(aBucketName, HEAD, &lWrapper, 0, 0, lEscapedKey, 0);
  } catch (AWSException& e) {
    lWrapper.destroyParser();
    curl_free(lEscapedKeyChar);
    throw e;
  }

  lWrapper.destroyParser();

  curl_free(lEscapedKeyChar);

  if ( ! lRes->isSuccessful() )
    throw HeadException( lRes->theS3ResponseError );

  return lRes.release();
}

void
S3Connection::setRequestMethod(ActionType aActionType)
{
  curl_easy_setopt(theCurl, CURLOPT_READFUNCTION, setCreateBucketData);
  // this is overriden in the curlstreambuf
  curl_easy_setopt(theCurl, CURLOPT_WRITEFUNCTION,  S3Connection::getS3Data);
  curl_easy_setopt(theCurl, CURLOPT_FRESH_CONNECT, "FALSE");
  switch (aActionType) {
      case CREATE_BUCKET: {
          curl_easy_setopt(theCurl, CURLOPT_READFUNCTION, S3Connection::setCreateBucketData);
          curl_easy_setopt(theCurl, CURLOPT_CUSTOMREQUEST, 0);
          curl_easy_setopt(theCurl, CURLOPT_UPLOAD, 1);
          curl_easy_setopt(theCurl, CURLOPT_HTTPGET, 0);
          break;
      }
      case LIST_ALL_BUCKETS: {
          curl_easy_setopt(theCurl, CURLOPT_CUSTOMREQUEST, 0);
          curl_easy_setopt(theCurl, CURLOPT_HTTPGET, 1);
          curl_easy_setopt(theCurl, CURLOPT_UPLOAD, 0);
          break;
      }
      case LIST_BUCKET: {
          curl_easy_setopt(theCurl, CURLOPT_CUSTOMREQUEST, 0);
          curl_easy_setopt(theCurl, CURLOPT_HTTPGET, 1);
          curl_easy_setopt(theCurl, CURLOPT_UPLOAD, 0);
          break;
      }
      case DELETE_BUCKET: {
          curl_easy_setopt(theCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
          curl_easy_setopt(theCurl, CURLOPT_UPLOAD, 0);
          curl_easy_setopt(theCurl, CURLOPT_HTTPGET, 0);
          break;
      }
      case PUT: {
          curl_easy_setopt(theCurl, CURLOPT_READFUNCTION, S3Connection::setPutData);
          curl_easy_setopt(theCurl, CURLOPT_CUSTOMREQUEST, 0);
          curl_easy_setopt(theCurl, CURLOPT_HTTPGET, 0);
          curl_easy_setopt(theCurl, CURLOPT_UPLOAD, 1);
          break;
      }
      case GET: {
          curl_easy_setopt(theCurl, CURLOPT_CUSTOMREQUEST, 0);
          curl_easy_setopt(theCurl, CURLOPT_HTTPGET, 1);
          curl_easy_setopt(theCurl, CURLOPT_UPLOAD, 0);
          break;
      }
      case HEAD: {
          curl_easy_setopt(theCurl, CURLOPT_CUSTOMREQUEST, "HEAD");
          curl_easy_setopt(theCurl, CURLOPT_UPLOAD, 0);
          curl_easy_setopt(theCurl, CURLOPT_HTTPGET, 0);
          break;
      }
      case DELETE: {
          curl_easy_setopt(theCurl, CURLOPT_CUSTOMREQUEST, "DELETE");
          curl_easy_setopt(theCurl, CURLOPT_UPLOAD, 0);
          curl_easy_setopt(theCurl, CURLOPT_HTTPGET, 0);
          break;
      }
      default: {
          assert(false);
      }
  }
}

void
S3Connection::makeRequest(const std::string& aBucketName,
                          ActionType aActionType, S3CallBackWrapper* aCallBackWrapper,
                          PathArgs_t* aPathArgsMap, RequestHeaderMap* aHeaderMap)
{
  makeRequest(aBucketName, aActionType, aCallBackWrapper, aPathArgsMap, aHeaderMap, "", 0);
}

void
S3Connection::makeRequest(const std::string& aBucketName,
    ActionType aActionType, S3CallBackWrapper* aCallBackWrapper,
    PathArgs_t* aPathArgsMap, RequestHeaderMap* aHeaderMap,
    const std::string& aKey, S3Object* aObject)
{
  S3Response* lResponse;
  aws::CallingFormat* lCallingFormat;
  RequestHeaderMap lHeaderMap;
  std::string lStringToSign;
  std::stringstream lAuthData;
  CURLcode lResCode;
  struct curl_slist* lSList;

  lResponse = aCallBackWrapper->theResponse;
  lCallingFormat = aws::CallingFormat::getRegularCallingFormat();
  std::string lUrl = lCallingFormat->getUrl(theIsSecure, theHost, thePort,
                                            aBucketName, aKey, aPathArgsMap);

  // set the request url
  curl_easy_setopt(theCurl, CURLOPT_URL, lUrl.c_str());

  // set the request method (i.e. get, put) and the according callback functions
  setRequestMethod(aActionType);

  // set the data object received in the callback function
  curl_easy_setopt(theCurl, CURLOPT_WRITEDATA, (void*)(aCallBackWrapper));
  curl_easy_setopt(theCurl, CURLOPT_WRITEHEADER, (void*)(aCallBackWrapper));

  // if we didn't get an existing header map as input, we have to create one
  if (!aHeaderMap)
      aHeaderMap = &lHeaderMap;

  // add the date header according to the S3 REST spec
  aHeaderMap->addDateHeader();

  if (aObject) {
    curl_easy_setopt(theCurl, CURLOPT_READDATA, (void*) aObject);
    aHeaderMap->addMetadataHeaders(aObject);
    aHeaderMap->addHeader("Content-Type", aObject->theContentType);
    curl_easy_setopt(theCurl, CURLOPT_INFILESIZE, aObject->theContentLength);
    aHeaderMap->addHeader("Transfer-Encoding", "");
    aHeaderMap->addHeader("Expect", "");
  } else {
    curl_easy_setopt(theCurl, CURLOPT_READDATA, 0);
    curl_easy_setopt(theCurl, CURLOPT_INFILESIZE, 0);
  }

  // authorization
  lStringToSign = Canonizer::canonicalize(aActionType, aBucketName, aKey,
      aHeaderMap);

  {
    // compute signature
    HMAC(EVP_sha1(), theSecretAccessKey.c_str(),  theSecretAccessKey.size(),
        (const unsigned char*) lStringToSign.c_str(), lStringToSign.size(),
        theEncryptedResult, &theEncryptedResultSize);

    long lBase64EncodedStringLength;
    lAuthData << " AWS " << theAccessKeyId << ":" <<
        base64Encode(theEncryptedResult, theEncryptedResultSize,
                     lBase64EncodedStringLength);
  }


  // avoid temporary objects
  std::string lAuthDataString = lAuthData.str();
  aHeaderMap->addHeader("Authorization", lAuthDataString.c_str());

  lSList = 0;

  // add headers to curl
  aHeaderMap->addHeadersToCurlSList(lSList);

  curl_easy_setopt(theCurl, CURLOPT_HTTPHEADER, lSList);

//  curl_easy_setopt(theCurl, CURLOPT_VERBOSE, 1);

  if (++theNumberOfRequests >= MAX_REQUESTS) {
    curl_easy_setopt(theCurl, CURLOPT_FRESH_CONNECT, "TRUE");
    theNumberOfRequests = 0;
  }

  GetResponse* lGetResponse = dynamic_cast<GetResponse*>(lResponse);
  if (lGetResponse) {
    lGetResponse->theStreamBuffer = new CurlStreamBuffer(theCurl);
    lGetResponse->theInputStream =
        new std::istream(lGetResponse->theStreamBuffer);
    lResCode = (CURLcode) lGetResponse->theStreamBuffer->multi_perform();

    // parse the error in case we had one
    if ( ! lResponse->isSuccessful() ) {
      char lBuf[1024];
      size_t lRead;
      while ( true ) {
        lGetResponse->theInputStream->read(lBuf, 1024);
        lRead = lGetResponse->theInputStream->gcount();
        if (lRead == 0) {
          break;
        }
        lBuf[lRead] = 0;
        xmlParseChunk(aCallBackWrapper->theParserCtxt, lBuf, lRead, 0);
      }
      xmlParseChunk(aCallBackWrapper->theParserCtxt, 0, 0, 1);
    }
  } else {
    lResCode = curl_easy_perform(theCurl);
    if (! (lResponse->isSuccessful()) ) {
      // tell the parser that parsing is finished
      xmlParseChunk(aCallBackWrapper->theParserCtxt, 0, 0, 1);
    }
  }
  curl_slist_free_all(lSList);

  if (lResCode != 0 && 
  !(lResCode==18 && !lGetResponse) // head only (reporting partial file, that can be ignored)
    ) {
     std::cerr << "[S3Connection::makeRequest] Response CURLCode is: " << (int) lResCode << std::endl;
    throw AWSConnectionException(theCurlErrorBuffer);
  }

}

size_t
S3Connection::getS3Data(void *ptr, size_t size, size_t nmemb, void *data)
{
  S3CallBackWrapper* lWrapper = static_cast<S3CallBackWrapper*>(data);

  char* lChars = static_cast<char*>(ptr);

  //std::cerr.write(lChars, size*nmemb);

  // this guarantees to read the input in chunks as they come in
  // by libxml; we always read as much as is in the buffer
  // because we stream internally.
  xmlParseChunk(lWrapper->theParserCtxt, lChars, size * nmemb, 0);

  return size * nmemb;
}

size_t
S3Connection::getHeaderData(void *ptr, size_t size, size_t nmemb, void *stream)
{
  S3CallBackWrapper* lWrapper = static_cast<S3CallBackWrapper*>(stream);
  S3Response* lRes = lWrapper->theResponse;
  GetResponse* lGetResponse = 0;
  HeadResponse* lHeadResponse = 0;
  CreateBucketResponse* lCreateResponse = 0;

  std::string lTmp(static_cast<char*>(ptr), size*nmemb);
  trim(lTmp);

  if ((lHeadResponse = dynamic_cast<HeadResponse*>(lRes))) {
    if (lTmp.find("404 Not") != std::string::npos) {

      lHeadResponse->theIsSuccessful = false;
      lHeadResponse->theS3ResponseError.theErrorCode=S3Exception::NoSuchKey;
      lHeadResponse->theS3ResponseError.theErrorMessage="NOT FOUND";
    }else if ( lTmp.find("Content-Length:") != std::string::npos) {
      lHeadResponse->theContentLength = atoll(lTmp.c_str() + 16);
    } else if ( lTmp.find("Content-Type:") != std::string::npos) {
      lHeadResponse->theContentType = lTmp.substr(14, lTmp.length() -14);
    }
  }

  if (lTmp.find("200 OK") != std::string::npos ||
      lTmp.find("204 No Content") != std::string::npos) {
    // if we got a 20x header, the request was successful
    lRes->theIsSuccessful = true;
  } else if (lTmp.find("ETag:") != std::string::npos) {
    lRes->theETag = lTmp.substr(7, lTmp.find_last_of('"') - 7);
  } else if (lTmp.find("Date:") != std::string::npos) {
    lRes->theDate = lTmp.substr(6, lTmp.size());
  } else if (lTmp.find("x-amz-id-2:") != std::string::npos) {
    lRes->theAmazonId = lTmp.substr(12, lTmp.size());
  } else if (lTmp.find("x-amz-request-id:") != std::string::npos) {
    lRes->theRequestId = lTmp.substr(18, lTmp.size());
  } else if (lTmp.find("x-amz-meta-") != std::string::npos) {
    size_t lEndOfName = lTmp.find_first_of(":")+1;
    assert (lEndOfName != std::string::npos);
    std::string lName = lTmp.substr(11, lEndOfName - 12);
    std::string lValue = lTmp.substr(lEndOfName+1, lTmp.length());
    lRes->theMetaData.insert(std::pair<std::string, std::string>(lName, lValue));
  } else if ((lGetResponse = dynamic_cast<GetResponse*>(lRes))) {
    if (lTmp.find("Last-Modified:") != std::string::npos) {
      // parse a time string of the following format: Fri, 09 Nov 2007 13:05:49 GMT
      Time t(lTmp.c_str()+15);
      lGetResponse->theLastModified = t;

    } else if ( lTmp.find("Content-Length:") != std::string::npos) {
      lGetResponse->theContentLength = atoll(lTmp.c_str() + 16);
    } else if ( lTmp.find("Content-Type:") != std::string::npos) {
      lGetResponse->theContentType = lTmp.substr(14, lTmp.length() -14);
    } else if ( lTmp.find("304 N") != std::string::npos ) {
      // not modified (returned when using If-Modified-Since or If-Non-Match)
      lGetResponse->theIsSuccessful = true;
      lGetResponse->theIsModified = false;
    }
  } else if ((lCreateResponse = dynamic_cast<CreateBucketResponse*>(lRes))) {
    if (lTmp.find("Location:") != std::string::npos) {
      lCreateResponse->theLocation = lTmp.substr(10, lTmp.find_last_of('"') - 10);
    }
  }

  return size * nmemb;
}


size_t
S3Connection::setCreateBucketData(void *aBuffer, size_t aSize, size_t nmemb, void *stream)
{
  return 0; // signal EOF
}

size_t
S3Connection::setPutData(void *aBuffer, size_t aSize, size_t nmemb, void* data)
{
  S3Object* lObject = static_cast<S3Object*>(data);
  size_t maxsize = aSize * nmemb;
  size_t remaining;
  char* charptr = static_cast<char*>(aBuffer);

  if (lObject->theIstream) { // serve data from an input steram
    std::istream* in = lObject->theIstream;
    in->read(charptr, maxsize);
    return in->gcount();
  }
  else if (lObject->theDataPointer) { // serve data from a char pointer
    remaining = lObject->theContentLength - lObject->theDataRead;
    remaining = std::min(remaining, maxsize);
    memcpy(aBuffer, lObject->theDataPointer + lObject->theDataRead, remaining);
    lObject->theDataRead += remaining;
    return remaining;
  }
  else {
    assert(false);  // either of the two (theIstream or theDataPointer) must be set
  }
  return 0; // avoid warning
}

std::string
S3Connection::requestTypeForAction(ActionType aType)
{
  switch (aType) {
     case LIST_ALL_BUCKETS: {
         return "GET";
      }
      case LIST_BUCKET: {
          return "GET";
      }
      case CREATE_BUCKET: {
          return "PUT";
      }
      case DELETE_BUCKET: {
          return "DELETE";
      }
      case PUT: {
          return "PUT";
      }
      case GET: {
          return "GET";
      }
      case DELETE: {
          return "DELETE";
      }
      case HEAD: {
          return "HEAD";
      }
      default: {
          assert(false);
      }
  }
  return "";
}

} } // namespaces

