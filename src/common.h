#ifndef AWS_COMMON_H
#define AWS_COMMON_H

#include <libaws/common.h>
#include <map>
#include <string.h>


#ifdef HAVE_INTTYPES_H
# include <inttypes.h>
#endif
#ifdef HAVE_STDINT_H
# include <stdint.h>
#endif

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
  } /* namespace s3 */


} /* namespace aws */
#endif
