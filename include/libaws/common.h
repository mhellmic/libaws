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
#ifndef LIBAWS_COMMON_API_H
#define LIBAWS_COMMON_API_H

#include <libaws/config.h>
#include <libaws/smart_ptr.h>

namespace aws {

  /**
   * S3 stuff
   */
  class S3Connection;
  typedef SmartPtr<S3Connection> S3ConnectionPtr;

  template <class T> class S3Response;
  typedef SmartPtr<S3Response<class T> > S3ResponsePtr;

  class CreateBucketResponse;
  typedef SmartPtr<CreateBucketResponse> CreateBucketResponsePtr;

  class ListAllBucketsResponse;
  typedef SmartPtr<ListAllBucketsResponse> ListAllBucketsResponsePtr;

  class ListBucketResponse;
  typedef SmartPtr<ListBucketResponse> ListBucketResponsePtr;

  class DeleteBucketResponse;
  typedef SmartPtr<DeleteBucketResponse> DeleteBucketResponsePtr;

  class PutResponse;
  typedef SmartPtr<PutResponse> PutResponsePtr;

  class GetResponse;
  typedef SmartPtr<GetResponse> GetResponsePtr;

  class DeleteResponse;
  typedef SmartPtr<DeleteResponse> DeleteResponsePtr;

  class DeleteAllResponse;
  typedef SmartPtr<DeleteAllResponse> DeleteAllResponsePtr;

  class HeadResponse;
  typedef SmartPtr<HeadResponse> HeadResponsePtr;

  /**
   * SQS stuff
   */
  class SQSConnection;
  typedef SmartPtr<SQSConnection> SQSConnectionPtr;

  template <class T> class SQSResponse;
  typedef SmartPtr<SQSResponse<class T> > SQSResponsePtr;

  class CreateQueueResponse;
  typedef SmartPtr<CreateQueueResponse> CreateQueueResponsePtr;

  class DeleteQueueResponse;
  typedef SmartPtr<DeleteQueueResponse> DeleteQueueResponsePtr;

  class ListQueuesResponse;
  typedef SmartPtr<ListQueuesResponse> ListQueuesResponsePtr;

  class SendMessageResponse;
  typedef SmartPtr<SendMessageResponse> SendMessageResponsePtr;

  class ReceiveMessageResponse;
  typedef SmartPtr<ReceiveMessageResponse> ReceiveMessageResponsePtr;

  class DeleteMessageResponse;
  typedef SmartPtr<DeleteMessageResponse> DeleteMessageResponsePtr;

  /**
   * SDB stuff
   */
  class SDBConnection;
  typedef SmartPtr<SDBConnection> SDBConnectionPtr;

  class SDBResponse;
  typedef SmartPtr<SDBResponse> SDBResponsePtr;

  class CreateDomainResponse;
  typedef SmartPtr<CreateDomainResponse> CreateDomainResponsePtr;

  class DeleteDomainResponse;
  typedef SmartPtr<DeleteDomainResponse> DeleteDomainResponsePtr;

  class DomainMetadataResponse;
  typedef SmartPtr<DomainMetadataResponse> DomainMetadataResponsePtr;

  class ListDomainsResponse;
  typedef SmartPtr<ListDomainsResponse> ListDomainsResponsePtr;

  class PutAttributesResponse;
  typedef SmartPtr<PutAttributesResponse> PutAttributesResponsePtr;

  class BatchPutAttributesResponse;
  typedef SmartPtr<BatchPutAttributesResponse> BatchPutAttributesResponsePtr;

  class DeleteAttributesResponse;
  typedef SmartPtr<DeleteAttributesResponse> DeleteAttributesResponsePtr;

  class GetAttributesResponse;
  typedef SmartPtr<GetAttributesResponse> GetAttributesResponsePtr;

  class SDBQueryResponse;
  typedef SmartPtr<SDBQueryResponse> SDBQueryResponsePtr;

  class SDBQueryWithAttributesResponse;
  typedef SmartPtr<SDBQueryWithAttributesResponse> SDBQueryWithAttributesResponsePtr;

} /* namespace aws */

#endif
