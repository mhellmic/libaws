#ifndef AWS_RESPONSE_H
#define AWS_RESPONSE_H

#include "common.h"

namespace aws { 
  namespace s3 {
    class CreateBucketHandler;
    class ListAllBucketsHandler;
    class DeleteBucketHandler;
    class ListBucketHandler;
    class PutHandler;
    class GetHandler;
    class DeleteHandler;
    class HeadHandler;
  } /* namespace s3 */
	
  class Response 
  {

    friend class aws::s3::S3Connection;
    friend class aws::s3::CreateBucketHandler;
    friend class aws::s3::ListAllBucketsHandler;
    friend class aws::s3::DeleteBucketHandler;
    friend class aws::s3::ListBucketHandler;
    friend class aws::s3::PutHandler;
    friend class aws::s3::GetHandler;
    friend class aws::s3::DeleteHandler;
    friend class aws::s3::HeadHandler;

  public:
    Response();
    virtual       ~Response();

    bool          isSuccessful() { return theIsSuccessful; }

  protected:
    bool 	        theIsSuccessful;
  };
    
} /* namespace aws */
#endif
