#include "common.h"

#include <libaws/s3exception.h>
#include "s3/s3response.h"
  
namespace aws {

  S3Exception::S3Exception(const s3::S3ResponseError& aError)
  {
    theErrorCode    = aError.getErrorCode();
    theErrorMessage = aError.getErrorMessage();
    theRequestId    = aError.getRequestId();
    theHostId       = aError.getHostId();
  }

  S3Exception::~S3Exception() throw() {}

  const char*
  S3Exception::what() const throw()
  {
   return theErrorMessage.c_str(); 
  }

  CreateBucketException::CreateBucketException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  CreateBucketException::~CreateBucketException() throw() {}

  DeleteBucketException::DeleteBucketException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  DeleteBucketException::~DeleteBucketException() throw() {}

  ListAllBucketsException::ListAllBucketsException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  ListAllBucketsException::~ListAllBucketsException() throw() {}

  ListBucketException::ListBucketException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  ListBucketException::~ListBucketException() throw() {}

  GetException::GetException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  GetException::~GetException() throw() {}

  PutException::PutException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  PutException::~PutException() throw() {}

  HeadException::HeadException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  HeadException::~HeadException() throw() {}

  DeleteException::DeleteException(const s3::S3ResponseError& aError)
  : S3Exception(aError) {}

  DeleteException::~DeleteException() throw() {}

} /* namespace aws */
