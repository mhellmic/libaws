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
