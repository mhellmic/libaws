#include <sstream>

#include "common.h"
#include "canonizer.h"

#include "s3/s3connection.h"
#include "requestheadermap.h"
#include "callingformat.h"

namespace aws { 

std::string
Canonizer::canonicalize(s3::S3Connection::ActionType aType, 
                        std::string aBucketName, std::string aKey,
                        RequestHeaderMap* aHeaderMap, bool aAclParam, 
                        bool aTorrentParam, bool aLoggingParam) {

    std::stringstream lStringToSign;
    
    lStringToSign << s3::S3Connection::requestTypeForAction(aType) << "\n";
    aHeaderMap->getHeaderStringToSign(&lStringToSign);
    
    // TODO repace with the help of the callingformat class
    // build the path using the bucket and key
    if (aBucketName.size() != 0) {
        lStringToSign << "/" << aBucketName;
    }
    // append the key (it might be an empty string)
    // append a slash regardless
    lStringToSign << "/";
    if(aKey.size() != 0) {
        lStringToSign << aKey;
    }
    
    // add params
    // TODO check that only one of the parameters is true (xor?)
    if (aAclParam) {
        lStringToSign << "?acl";
    } else if (aTorrentParam) {
        lStringToSign << "?torrent";
    } if (aLoggingParam) {
        lStringToSign << "?logging";
    }
    
    return lStringToSign.str();
}


std::string
Canonizer::convertPathArgs(PathArgs_t* aPathArgs)
{
    std::stringstream s;
    
    bool lFirstRun = true;
    
    if (aPathArgs)
    {
        for (PathArgs_t::iterator lIter = aPathArgs->begin(); 
             lIter != aPathArgs->end(); ++lIter) 
        {
            if (lFirstRun) {
                lFirstRun = false; 
                s << "?";
            } else {
                s << "&";
            } 

            s << (*lIter).first << "=" << (*lIter).second;
        }
    }
    
    return s.str();
}

}  // end namespaces

