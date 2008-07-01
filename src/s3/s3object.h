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
#ifndef AWS_S3OBJECT_H
#define AWS_S3OBJECT_H

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
