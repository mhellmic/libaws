#ifndef AWS_S3OBJECT_H
#define AWST_S3OBJECT_H

#include "common.h"

#include <map>
#include <list>
#include <istream>

namespace aws { namespace s3 
{
    
class S3Object 
{
public:
    S3Object();
    typedef std::map<std::string, std::list<std::string> > metadata_t;
    typedef metadata_t::iterator metadataiterator_t;
    typedef std::pair<std::string, std::list<std::string> > metadatapair_t;

    std::string      theContentType;
    size_t           theContentLength;
    metadata_t       theMetadata;

    // use either of the following memebers
    const char*      theDataPointer;
    std::istream*    theIstream;

    // data needed in the setPutData function
    size_t           theDataRead;
}; 
    
} } // end namespaces

#endif
