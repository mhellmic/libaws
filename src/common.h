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
#ifndef AWS_COMMON_H
#define AWS_COMMON_H

#include <libaws/common.h>
#include <map>
#include <string>


#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

#include "logging/logging.hh"

namespace aws { 
	
  typedef std::pair<std::string, std::string> stringpair_t;
  
  namespace s3
  {
    class S3Connection;
    class S3Response;
    class CreateBucketResponse;
    class ListAllBucketsResponse;
    class ListBucketResponse;
    class DeleteBucketResponse;
    class PutResponse;
    class GetResponse;
    class HeadResponse;
    class DeleteResponse;
    class DeleteAllResponse;
  } /* namespace s3 */


} /* namespace aws */
#endif
