#ifndef LIBAWS_COMMON_API_H
#define LIBAWS_COMMON_API_H

#include <libaws/config.h>
#include <libaws/smart_ptr.h>

namespace aws {

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

  class HeadResponse;
  typedef SmartPtr<HeadResponse> HeadResponsePtr;

} /* namespace aws */

#endif
