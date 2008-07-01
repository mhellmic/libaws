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

#include <time.h>
#include <cassert>

#include "s3/s3handler.h"
#include "s3/s3response.h"
#include "s3/s3callbackwrapper.h"


#include <iostream>
namespace aws { namespace s3 {

S3Handler::S3Handler()
  : theCurrentState(0)
{}

/**
 * CreateBucketHandler
 */
CreateBucketHandler::CreateBucketHandler()
  : S3Handler()
{}

void
CreateBucketHandler::startElementNs( void * ctx, 
                                    const xmlChar * localname, 
                                    const xmlChar * prefix, 
                                    const xmlChar * URI, 
                                    int nb_namespaces, 
                                    const xmlChar ** namespaces, 
                                    int nb_attributes, 
                                    int nb_defaulted, 
                                    const xmlChar ** attributes )
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  CreateBucketResponse* lRes     = static_cast<CreateBucketResponse*>( lWrapper->theResponse );
  CreateBucketHandler*  lHandler = static_cast<CreateBucketHandler*>(lWrapper->theHandler);
    
  if (xmlStrEqual(localname, BAD_CAST "Error")) {
    lRes->theIsSuccessful = false;
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Code")) {
    lHandler->setState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
    lHandler->setState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
    lHandler->setState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
    lHandler->setState(HostId);
  }
}
    
void
CreateBucketHandler::charactersSAXFunc(void * ctx, 
    					              const xmlChar * value, 
    					              int len)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  CreateBucketResponse* lRes     = static_cast<CreateBucketResponse*>( lWrapper->theResponse );
  CreateBucketHandler*  lHandler = static_cast<CreateBucketHandler*>(lWrapper->theHandler);

  if (lHandler->isSet(Code)) {
    lRes->theS3ResponseError.theErrorCode = 
    S3ResponseError::parseError(std::string((const char*)value, len));
  } 
  else if (lHandler->isSet(Message)) {
    lRes->theS3ResponseError.theErrorMessage = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(RequestId)) {
    lRes->theS3ResponseError.theRequestId = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(HostId)) {
    lRes->theS3ResponseError.theHostId = std::string((const char*)value, len);         
  }
}

void
CreateBucketHandler::endElementNs(void * ctx, 
    					         const xmlChar * localname, 
    					         const xmlChar * prefix, 
    					         const xmlChar * URI)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  CreateBucketHandler*  lHandler = static_cast<CreateBucketHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Code")) {
    lHandler->unsetState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
    lHandler->unsetState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
    lHandler->unsetState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
    lHandler->unsetState(HostId);
  }
}

/**
 * DeleteBucketHandler
 */

DeleteBucketHandler::DeleteBucketHandler()
        : S3Handler()
{}

void
DeleteBucketHandler::startElementNs( void * ctx, 
                                    const xmlChar * localname, 
                                    const xmlChar * prefix, 
                                    const xmlChar * URI, 
                                    int nb_namespaces, 
                                    const xmlChar ** namespaces, 
                                    int nb_attributes, 
                                    int nb_defaulted, 
                                    const xmlChar ** attributes )
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  DeleteBucketResponse* lRes     = static_cast<DeleteBucketResponse*>( lWrapper->theResponse );
  DeleteBucketHandler*  lHandler = static_cast<DeleteBucketHandler*>(lWrapper->theHandler);
    
  if (xmlStrEqual(localname, BAD_CAST "Error")) {
      lRes->theIsSuccessful = false;
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->setState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->setState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
      lHandler->setState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
      lHandler->setState(HostId);
  }
}
    
void
DeleteBucketHandler::charactersSAXFunc(void * ctx, 
    					              const xmlChar * value, 
    					              int len)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  DeleteBucketResponse* lRes     = static_cast<DeleteBucketResponse*>( lWrapper->theResponse );
  DeleteBucketHandler*  lHandler = static_cast<DeleteBucketHandler*>(lWrapper->theHandler);

  if (lHandler->isSet(Code)) {
      lRes->theS3ResponseError.theErrorCode = S3ResponseError::parseError(std::string((const char*)value, len));
  } 
  else if (lHandler->isSet(Message)) {
      lRes->theS3ResponseError.theErrorMessage = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(RequestId)) {
      lRes->theS3ResponseError.theRequestId = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(HostId)) {
      lRes->theS3ResponseError.theHostId = std::string((const char*)value, len);
  }
    
}

void
DeleteBucketHandler::endElementNs(void * ctx, 
    					         const xmlChar * localname, 
    					         const xmlChar * prefix, 
    					         const xmlChar * URI)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  //DeleteBucketResponse* lRes     = static_cast<DeleteBucketResponse*>( lWrapper->theResponse );
  DeleteBucketHandler*  lHandler = static_cast<DeleteBucketHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->unsetState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->unsetState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
      lHandler->unsetState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
      lHandler->unsetState(HostId);
  }
}


ListAllBucketsHandler::ListAllBucketsHandler()
        : S3Handler()
{}


void
ListAllBucketsHandler::startElementNs( void * ctx, 
                                    const xmlChar * localname, 
                                    const xmlChar * prefix, 
                                    const xmlChar * URI, 
                                    int nb_namespaces, 
                                    const xmlChar ** namespaces, 
                                    int nb_attributes, 
                                    int nb_defaulted, 
                                    const xmlChar ** attributes )
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  ListAllBucketsResponse* lRes     = static_cast<ListAllBucketsResponse*>( lWrapper->theResponse );
  ListAllBucketsHandler*  lHandler = static_cast<ListAllBucketsHandler*>(lWrapper->theHandler);
    
  if (xmlStrEqual(localname, BAD_CAST "ListAllMyBucketsResult")) {
      lRes->theIsSuccessful = true;
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Owner")) {
      lHandler->setState(Owner);
  }
  else if (xmlStrEqual(localname, BAD_CAST "Id")) {
      lHandler->setState(Id);
  }
  else if (xmlStrEqual(localname, BAD_CAST "DisplayName")) {
      lHandler->setState(DisplayName);
  }
  else if (xmlStrEqual(localname, BAD_CAST "Buckets")) {
      lHandler->setState(Buckets);
  }
  else if (xmlStrEqual(localname, BAD_CAST "Bucket")) {
      lHandler->setState(Bucket);
      ListAllBucketsResponse::Bucket lBucket;
      lRes->theBuckets.push_back(lBucket);
  }
  else if (xmlStrEqual(localname, BAD_CAST "Name")) {
      lHandler->setState(Name);
  }
  else if (xmlStrEqual(localname, BAD_CAST "CreationDate")) {
      lHandler->setState(CreationDate);
  }
  else if (xmlStrEqual(localname, BAD_CAST "Error")) {
      lRes->theIsSuccessful = false;
      lHandler->setState(Error);
  } 
  else if (lHandler->isSet(Error) && xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->setState(Code);
  } 
  else if (lHandler->isSet(Error) && xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->setState(Message);
  }
}

void
ListAllBucketsHandler::charactersSAXFunc(void * ctx, 
    					              const xmlChar * value, 
    					              int len)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  ListAllBucketsResponse* lRes     = static_cast<ListAllBucketsResponse*>( lWrapper->theResponse );
  ListAllBucketsHandler*  lHandler = static_cast<ListAllBucketsHandler*>(lWrapper->theHandler);
    
            
  if (lHandler->isSet(Owner) && lHandler->isSet(Id)) {
      lRes->theOwnerId = std::string((const char*)value, len);
  } 
  else if (lHandler->isSet(Owner) && lHandler->isSet(DisplayName)) {
      lRes->theOwnerDisplayName = std::string((const char*)value, len);
  } 
  else if (lHandler->isSet(Bucket) && lHandler->isSet(Name)) {
      ListAllBucketsResponse::Bucket& lBucket = lRes->theBuckets.back();
      lBucket.BucketName = std::string((const char*)value, len);
  } 
  else if (lHandler->isSet(Bucket) && lHandler->isSet(CreationDate)) {
      ListAllBucketsResponse::Bucket& lBucket = lRes->theBuckets.back();
      lBucket.CreationDate = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(Error) && lHandler->isSet(Code)) {
      lRes->theS3ResponseError.theErrorCode = S3ResponseError::parseError(std::string((const char*)value, len));      
  }
  else if (lHandler->isSet(Error) && lHandler->isSet(Message)) {
      lRes->theS3ResponseError.theErrorMessage = std::string((const char*)value, len);        
  }
}

void
ListAllBucketsHandler::endElementNs(void * ctx, 
    					         const xmlChar * localname, 
    					         const xmlChar * prefix, 
    					         const xmlChar * URI)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  //ListAllBucketsResponse* lRes     = static_cast<ListAllBucketsResponse*>( lWrapper->theResponse );
  ListAllBucketsHandler*  lHandler = static_cast<ListAllBucketsHandler*>(lWrapper->theHandler);
 
  if (xmlStrEqual(localname, BAD_CAST "Owner")) {
      lHandler->unsetState(Owner);
  }
  else if (xmlStrEqual(localname, BAD_CAST "Id")) {
      lHandler->unsetState(Id);
  }
  else if (xmlStrEqual(localname, BAD_CAST "DisplayName")) {
      lHandler->unsetState(DisplayName);
  }
  else if (xmlStrEqual(localname, BAD_CAST "Buckets")) {
      lHandler->unsetState(Buckets);
  }
  else if (xmlStrEqual(localname, BAD_CAST "Bucket")) {
      lHandler->unsetState(Bucket);
  }
  else if (xmlStrEqual(localname, BAD_CAST "Name")) {
      lHandler->unsetState(Name);
  }
  else if (xmlStrEqual(localname, BAD_CAST "CreationDate")) {
      lHandler->unsetState(CreationDate);
  }
  else if (xmlStrEqual(localname, BAD_CAST "Error")) {
      lHandler->unsetState(Error);
  } 
  else if (lHandler->isSet(Error) && xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->unsetState(Code);
  } 
  else if (lHandler->isSet(Error) && xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->unsetState(Message);
  }
}

ListBucketHandler::ListBucketHandler()
        : S3Handler()
{}


void
ListBucketHandler::startElementNs( void * ctx, 
                                    const xmlChar * localname, 
                                    const xmlChar * prefix, 
                                    const xmlChar * URI, 
                                    int nb_namespaces, 
                                    const xmlChar ** namespaces, 
                                    int nb_attributes, 
                                    int nb_defaulted, 
                                    const xmlChar ** attributes )
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  ListBucketResponse* lRes     = static_cast<ListBucketResponse*>( lWrapper->theResponse );
  ListBucketHandler*  lHandler = static_cast<ListBucketHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Error")) {
    lRes->theIsSuccessful = false;
  } else if (xmlStrEqual(localname, BAD_CAST "Code")) {
    lHandler->setState(Code);
  } else if (xmlStrEqual(localname, BAD_CAST "Message")) {
    lHandler->setState(Message);
  } else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
    lHandler->setState(RequestId);
  } else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
    lHandler->setState(HostId);
  } else if (xmlStrEqual(localname, BAD_CAST "Name")) {
    lHandler->setState(Name);
  } else if (xmlStrEqual(localname, BAD_CAST "Prefix")) {
    lHandler->setState(Prefix);
  } else if (xmlStrEqual(localname, BAD_CAST "Marker")) {
    lHandler->setState(Marker);
  } else if (xmlStrEqual(localname, BAD_CAST "IsTruncated")) {
    lHandler->setState(Truncated);
  } else if (xmlStrEqual(localname, BAD_CAST "Contents")) {
    lHandler->setState(Contents);
  } else if (xmlStrEqual(localname, BAD_CAST "Key")) {
    lHandler->setState(Key);
  } else if (xmlStrEqual(localname, BAD_CAST "LastModified")) {
    lHandler->setState(LastModified);
  } else if (xmlStrEqual(localname, BAD_CAST "ETag")) {
    lHandler->setState(ETag);
  } else if (xmlStrEqual(localname, BAD_CAST "Size")) {
    lHandler->setState(Length);
  } else if (xmlStrEqual(localname, BAD_CAST "CommonPrefixes")) {
    lHandler->setState(CommonPrefixes);
  }
}
    
void
ListBucketHandler::charactersSAXFunc(void * ctx, 
    					              const xmlChar * value, 
    					              int len)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  ListBucketResponse* lRes     = static_cast<ListBucketResponse*>( lWrapper->theResponse );
  ListBucketHandler*  lHandler = static_cast<ListBucketHandler*>(lWrapper->theHandler);
            
  if (lHandler->isSet(Code)) {
    lRes->theS3ResponseError.theErrorCode = S3ResponseError::parseError(std::string((const char*)value, len));
  } else if (lHandler->isSet(Message)) {
    lRes->theS3ResponseError.theErrorMessage = std::string((const char*)value, len);
  } else if (lHandler->isSet(RequestId)) {
    lRes->theS3ResponseError.theRequestId = std::string((const char*)value, len);
  } else if (lHandler->isSet(HostId)) {
    lRes->theS3ResponseError.theHostId = std::string((const char*)value, len);         
  } else if (lHandler->isSet(Truncated)) {
    lRes->theIsTruncated = ((std::string((const char*)value, len)).compare("true") == 0);
  } else if (lHandler->isSet(Contents) && lHandler->isSet(Key)) {
    ListBucketResponse::Key lKey;
    lKey.KeyValue = std::string((const char*)value, len);
    lRes->theKeys.push_back(lKey);
  } else if (lHandler->isSet(Contents) && lHandler->isSet(LastModified)) {
    // FIXME convert to tm or time_t
    ListBucketResponse::Key& lKey = lRes->theKeys.back();
    lKey.LastModified = std::string((const char*)value, len);
  } else if (lHandler->isSet(Contents) && lHandler->isSet(ETag)) {
    if (len != 1) {
      ListBucketResponse::Key& lKey = lRes->theKeys.back();
      lKey.ETag = std::string((const char*)value, len);
    }
  } else if (lHandler->isSet(Contents) && lHandler->isSet(Length)) {
    char** lEndValue = 0;
    ListBucketResponse::Key& lKey = lRes->theKeys.back();
    std::string lTmp((const char*)value, len);
#ifdef HAVE_STRTOIMAX_F
    lKey.Length = strtoimax(lTmp.c_str(), lEndValue, 10);
#else

#endif
#ifndef NDEBUG
    assert(lEndValue=='\0');
#endif
  } else if (lHandler->isSet(CommonPrefixes) && lHandler->isSet(Prefix)) {
    lRes->theCommonPrefixes.push_back(std::string((const char*)value, len));
  }
}

void
ListBucketHandler::endElementNs(void * ctx, 
    					         const xmlChar * localname, 
    					         const xmlChar * prefix, 
    					         const xmlChar * URI)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  // ListBucketResponse* lRes     = static_cast<ListBucketResponse*>( lWrapper->theResponse );
  ListBucketHandler*  lHandler = static_cast<ListBucketHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Code")) {
    lHandler->unsetState(Code);
  } else if (xmlStrEqual(localname, BAD_CAST "Message")) {
    lHandler->unsetState(Message);
  } else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
    lHandler->unsetState(RequestId);
  } else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
    lHandler->unsetState(HostId);
  } else if (xmlStrEqual(localname, BAD_CAST "Name")) {
    lHandler->unsetState(Name);
  } else if (xmlStrEqual(localname, BAD_CAST "Prefix")) {
    lHandler->unsetState(Prefix);
  } else if (xmlStrEqual(localname, BAD_CAST "Marker")) {
    lHandler->unsetState(Marker);
  } else if (xmlStrEqual(localname, BAD_CAST "IsTruncated")) {
    lHandler->unsetState(Truncated);
  } else if (xmlStrEqual(localname, BAD_CAST "Contents")) {
    lHandler->unsetState(Contents);
  } else if (xmlStrEqual(localname, BAD_CAST "Key")) {
    lHandler->unsetState(Key);
  } else if (xmlStrEqual(localname, BAD_CAST "LastModified")) {
    lHandler->unsetState(LastModified);
  } else if (xmlStrEqual(localname, BAD_CAST "ETag")) {
    lHandler->unsetState(ETag);
  } else if (xmlStrEqual(localname, BAD_CAST "Size")) {
    lHandler->unsetState(Length);
  } else if (xmlStrEqual(localname, BAD_CAST "CommonPrefixes")) {
    lHandler->unsetState(CommonPrefixes);
  }
}

PutHandler::PutHandler()
        : S3Handler()
{}


void
PutHandler::startElementNs( void * ctx, 
                            const xmlChar * localname, 
                            const xmlChar * prefix, 
                            const xmlChar * URI, 
                            int nb_namespaces, 
                            const xmlChar ** namespaces, 
                            int nb_attributes, 
                            int nb_defaulted, 
                            const xmlChar ** attributes )
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  PutResponse* lRes     = static_cast<PutResponse*>( lWrapper->theResponse );
  PutHandler*  lHandler = static_cast<PutHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Error")) {
      lRes->theIsSuccessful = false;
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->setState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->setState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
      lHandler->setState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
      lHandler->setState(HostId);
  }
}
    
void
PutHandler::charactersSAXFunc(void * ctx, 
    					              const xmlChar * value, 
    					              int len)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  PutResponse* lRes     = static_cast<PutResponse*>( lWrapper->theResponse );
  PutHandler*  lHandler = static_cast<PutHandler*>(lWrapper->theHandler);
            
  if (lHandler->isSet(Code)) {
      lRes->theS3ResponseError.theErrorCode = S3ResponseError::parseError(std::string((const char*)value, len));
  } 
  else if (lHandler->isSet(Message)) {
      lRes->theS3ResponseError.theErrorMessage = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(RequestId)) {
      lRes->theS3ResponseError.theRequestId = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(HostId)) {
      lRes->theS3ResponseError.theHostId = std::string((const char*)value, len);         
  }
}

void
PutHandler::endElementNs(void * ctx, 
    					         const xmlChar * localname, 
    					         const xmlChar * prefix, 
    					         const xmlChar * URI)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  // PutResponse* lRes     = static_cast<PutResponse*>( lWrapper->theResponse );
  PutHandler*  lHandler = static_cast<PutHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->unsetState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->unsetState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
      lHandler->unsetState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
      lHandler->unsetState(HostId);
  }
}

GetHandler::GetHandler()
    : S3Handler()
{
    
}


void
GetHandler::startElementNs( void * ctx, 
                            const xmlChar * localname, 
                            const xmlChar * prefix, 
                            const xmlChar * URI, 
                            int nb_namespaces, 
                            const xmlChar ** namespaces, 
                            int nb_attributes, 
                            int nb_defaulted, 
                            const xmlChar ** attributes )
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  GetResponse* lRes     = static_cast<GetResponse*>( lWrapper->theResponse );
  GetHandler*  lHandler = static_cast<GetHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Error")) {
      lRes->theIsSuccessful = false;
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->setState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->setState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
      lHandler->setState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
      lHandler->setState(HostId);
  }
}
    
void
GetHandler::charactersSAXFunc(void * ctx, 
    					              const xmlChar * value, 
    					              int len)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  GetResponse* lRes     = static_cast<GetResponse*>( lWrapper->theResponse );
  GetHandler*  lHandler = static_cast<GetHandler*>(lWrapper->theHandler);
            
  if (lHandler->isSet(Code)) {
      lRes->theS3ResponseError.theErrorCode = S3ResponseError::parseError(std::string((const char*)value, len));
  } 
  else if (lHandler->isSet(Message)) {
      lRes->theS3ResponseError.theErrorMessage = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(RequestId)) {
      lRes->theS3ResponseError.theRequestId = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(HostId)) {
      lRes->theS3ResponseError.theHostId = std::string((const char*)value, len);         
  }
}

void
GetHandler::endElementNs(void * ctx, 
    					         const xmlChar * localname, 
    					         const xmlChar * prefix, 
    					         const xmlChar * URI)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  // GetResponse* lRes     = static_cast<GetResponse*>( lWrapper->theResponse );
  GetHandler*  lHandler = static_cast<GetHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->unsetState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->unsetState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
      lHandler->unsetState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
      lHandler->unsetState(HostId);
  }
}


HeadHandler::HeadHandler()
    : S3Handler()
{
    
}


void
HeadHandler::startElementNs( void * ctx, 
                                    const xmlChar * localname, 
                                    const xmlChar * prefix, 
                                    const xmlChar * URI, 
                                    int nb_namespaces, 
                                    const xmlChar ** namespaces, 
                                    int nb_attributes, 
                                    int nb_defaulted, 
                                    const xmlChar ** attributes )
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  HeadResponse* lRes     = static_cast<HeadResponse*>( lWrapper->theResponse );
  HeadHandler*  lHandler = static_cast<HeadHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Error")) {
      lRes->theIsSuccessful = false;
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->setState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->setState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
      lHandler->setState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
      lHandler->setState(HostId);
  }
}
    
void
HeadHandler::charactersSAXFunc(void * ctx, 
    					              const xmlChar * value, 
    					              int len)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  HeadResponse* lRes     = static_cast<HeadResponse*>( lWrapper->theResponse );
  HeadHandler*  lHandler = static_cast<HeadHandler*>(lWrapper->theHandler);
            
  if (lHandler->isSet(Code)) {
      lRes->theS3ResponseError.theErrorCode = S3ResponseError::parseError(std::string((const char*)value, len));
  } 
  else if (lHandler->isSet(Message)) {
      lRes->theS3ResponseError.theErrorMessage = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(RequestId)) {
      lRes->theS3ResponseError.theRequestId = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(HostId)) {
      lRes->theS3ResponseError.theHostId = std::string((const char*)value, len);         
  }
}

void
HeadHandler::endElementNs(void * ctx, 
    					         const xmlChar * localname, 
    					         const xmlChar * prefix, 
    					         const xmlChar * URI)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  // HeadResponse* lRes     = static_cast<HeadResponse*>( lWrapper->theResponse );
  HeadHandler*  lHandler = static_cast<HeadHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->unsetState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->unsetState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
      lHandler->unsetState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
      lHandler->unsetState(HostId);
  }
}


DeleteHandler::DeleteHandler()
    : S3Handler()
{
    
}


void
DeleteHandler::startElementNs( void * ctx, 
                                    const xmlChar * localname, 
                                    const xmlChar * prefix, 
                                    const xmlChar * URI, 
                                    int nb_namespaces, 
                                    const xmlChar ** namespaces, 
                                    int nb_attributes, 
                                    int nb_defaulted, 
                                    const xmlChar ** attributes )
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  DeleteResponse* lRes     = static_cast<DeleteResponse*>( lWrapper->theResponse );
  DeleteHandler*  lHandler = static_cast<DeleteHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Error")) {
      lRes->theIsSuccessful = false;
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->setState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->setState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
      lHandler->setState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
      lHandler->setState(HostId);
  }
}
    
void
DeleteHandler::charactersSAXFunc(void * ctx, 
    					              const xmlChar * value, 
    					              int len)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  DeleteResponse* lRes     = static_cast<DeleteResponse*>( lWrapper->theResponse );
  DeleteHandler*  lHandler = static_cast<DeleteHandler*>(lWrapper->theHandler);
            
  if (lHandler->isSet(Code)) {
      lRes->theS3ResponseError.theErrorCode = S3ResponseError::parseError(std::string((const char*)value, len));
  } 
  else if (lHandler->isSet(Message)) {
      lRes->theS3ResponseError.theErrorMessage = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(RequestId)) {
      lRes->theS3ResponseError.theRequestId = std::string((const char*)value, len);
  }
  else if (lHandler->isSet(HostId)) {
      lRes->theS3ResponseError.theHostId = std::string((const char*)value, len);         
  }
}

void
DeleteHandler::endElementNs(void * ctx, 
    					         const xmlChar * localname, 
    					         const xmlChar * prefix, 
    					         const xmlChar * URI)
{
  S3CallBackWrapper*    lWrapper = static_cast<S3CallBackWrapper*>( ctx );
  // DeleteResponse* lRes     = static_cast<DeleteResponse*>( lWrapper->theResponse );
  DeleteHandler*  lHandler = static_cast<DeleteHandler*>(lWrapper->theHandler);

  if (xmlStrEqual(localname, BAD_CAST "Code")) {
      lHandler->unsetState(Code);
  } 
  else if (xmlStrEqual(localname, BAD_CAST "Message")) {
      lHandler->unsetState(Message);
  }
  else if (xmlStrEqual(localname, BAD_CAST "RequestId")) {
      lHandler->unsetState(RequestId);
  }
  else if (xmlStrEqual(localname, BAD_CAST "HostId")) {
      lHandler->unsetState(HostId);
  }
}

} } // end namespaces
