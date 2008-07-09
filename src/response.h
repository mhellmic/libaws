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
