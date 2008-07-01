#ifndef S3REST_CANONIZER_H
#define S3REST_CANONIZER_H

#include "common.h"

#include <map>

#include "s3/s3connection.h"

namespace aws { 

class RequestHeaderMap;

class Canonizer {
        
public:
    static std::string canonicalize(s3::S3Connection::ActionType aRequestMethod, 
                                    std::string aBucketName, std::string aKey,
                                    RequestHeaderMap* aHeaderMap, bool aAclParam = false, 
                                    bool aTorrentParam = false, bool aLoggingParam = false);
                                    
    static std::string convertPathArgs(PathArgs_t* aPathArgs); 
};

} // end namespace


#endif
