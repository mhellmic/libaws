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
#ifndef S3REST_REQUESTHEADERMAP_H
#define S3REST_REQUESTHEADERMAP_H

#include "common.h"

#include <map>
#include <string>
#include <sstream>
#include <cctype>
#include <algorithm>


struct curl_slist;

namespace aws { 
    
namespace s3 {
	class S3Object;
}

class RequestHeaderMap
{
  
  
public:
    typedef std::multimap<std::string, std::string> requestmap_t;
    typedef requestmap_t::iterator requestmapiter_t;
    
    
private:
    requestmap_t theMap;
    static std::string DATE_FORMAT;
    
public:
    void
    addHeader(std::string aKey, std::string aValue);
    
    bool
    containsKey(std::string aKey);
    
    void
    addDateHeader();
    
    void
    addHeadersToCurlSList(curl_slist*& aSList);
    
    void
    addMetadataHeaders(aws::s3::S3Object* aObject);

    void
    getHeaderStringToSign(std::stringstream* aStringToSign);
    
private:
    void
    trim(std::string& aStr);
    
    void
    replaceString(const std::string& aSearchString, 
                  const std::string& aReplaceString, 
                  std::string& aStringToReplace);
};

} // end namepsaces


#endif
