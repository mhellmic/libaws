#include "common.h"
#include "s3/s3object.h"
#include <istream>

namespace aws { namespace s3 
{
    
S3Object::S3Object()
    : theContentType("text/plain"),
      theContentLength(0),
      theDataPointer(0),
      theIstream(0),
      theDataRead(0)
{ }    
    
} } // end namespaces
