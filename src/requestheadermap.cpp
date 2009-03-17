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
#include "common.h"

#include "requestheadermap.h"

#include <iostream>
#include <sstream>
#include <map>
#include <time.h>
#include <cassert>
#include <curl/curl.h>

#include "awsconnection.h"

#include "s3/s3object.h"

namespace aws { 

std::string RequestHeaderMap::DATE_FORMAT("%a, %d %b %Y %H:%M:%S GMT");


void 
RequestHeaderMap::addHeader(std::string aKey, std::string aValue)
{
    theMap.insert(stringpair_t(aKey, aValue));
}
    
bool
RequestHeaderMap::containsKey(std::string aKey)
{
    return (theMap.count(aKey) != 0);
}

void
RequestHeaderMap::addDateHeader()
{
    time_t lRawTime; tm*    lPtm;
    time ( &lRawTime );
    lPtm = gmtime ( &lRawTime );

    char* lDateString = new char[31];
#ifndef NDEBUG
    size_t lTest = strftime(lDateString, 31, DATE_FORMAT.c_str(), lPtm);
    assert(lTest<31); // should be long enough for the date format above
#else
    strftime(lDateString, 31, DATE_FORMAT.c_str(), lPtm);
#endif

    addHeader("Date", lDateString);
    
    delete[] lDateString; lDateString = 0;
}

void
RequestHeaderMap::addMetadataHeaders(aws::s3::S3Object* aObject)
{
    
}

void
RequestHeaderMap::getHeaderStringToSign(std::stringstream* aStringToSign)
{
    requestmap_t lInterestMap;
 
    for (requestmapiter_t lIter = theMap.begin(); lIter != theMap.end(); ++lIter) {

        std::string lHashKey = (*lIter).first;
        std::transform(lHashKey.begin(), lHashKey.end(), lHashKey.begin(), (int(*)(int)) std::tolower);
        
        if ((lHashKey.find(AWSConnection::AMAZON_HEADER_PREFIX) == 0) // key starts with amazon_header_prefix
            || (lHashKey.compare("content-type") == 0)
            || (lHashKey.compare("content-md5") == 0)
            || (lHashKey.compare("expires") == 0)
            || (lHashKey.compare("date") == 0))
        {
            std::stringstream lConcatenatedList;
            int lCount = theMap.count((*lIter).first);
            for (requestmapiter_t lMultiIter = theMap.lower_bound((*lIter).first);
                 lMultiIter != theMap.upper_bound((*lIter).first); ++lMultiIter)
            {
                std::string lValue = (*lMultiIter).second;
                // replace \n with "" and trim
                replaceString("\n", "", lValue); trim(lValue);
                lConcatenatedList << lValue;
                if (--lCount > 0)
                {
                    lConcatenatedList << ",";
                }
            }
            lInterestMap.insert(stringpair_t(lHashKey, lConcatenatedList.str()));
        }
    }
    
    
    if ((lInterestMap.count(AWSConnection::ALTERNATIVE_DATE_HEADER) != 0) &&
        (lInterestMap.count("expires") != 0)) {  // contains key
        lInterestMap.insert(stringpair_t("date", ""));
    }

    if (lInterestMap.count("content-type") == 0) // does not contain key
    {
        lInterestMap.insert(stringpair_t("content-type", ""));
    }

    if (lInterestMap.count("content-md5") == 0) // does not contain key
    {
        lInterestMap.insert(stringpair_t("content-md5", ""));
    }
        
    for (requestmapiter_t lIter = lInterestMap.begin(); lIter != lInterestMap.end(); ++lIter)
    {
        std::string lHeaderKey = (*lIter).first;
        if (lHeaderKey.find(AWSConnection::AMAZON_HEADER_PREFIX) == 0) // starts with
        {
            *aStringToSign << lHeaderKey << ":" << (*lIter).second;
        }
        else
        {
            *aStringToSign <<  (*lIter).second;
        }
        *aStringToSign << "\n";
    }
    
}
    
void
RequestHeaderMap::trim(std::string& aStr)
{
 std::string::size_type lPos = aStr.find_last_not_of(' ');
 if(lPos != std::string::npos) {
   aStr.erase(lPos + 1);
   lPos = aStr.find_first_not_of(' ');
   if(lPos != std::string::npos) aStr.erase(0, lPos);
 }
 else
   aStr.erase(aStr.begin(), aStr.end());
}


void
RequestHeaderMap::replaceString(const std::string& aSearchString, 
                                const std::string& aReplaceString, 
                                std::string& aStringToReplace)
{
  std::string::size_type lPos = aStringToReplace.find(aSearchString, 0);
  int intLengthSearch = aSearchString.length();
  
  while(std::string::npos != lPos) {
    aStringToReplace.replace(lPos, intLengthSearch, aReplaceString);
    lPos = aStringToReplace.find(aSearchString, lPos + intLengthSearch);
  }
}

void
RequestHeaderMap::addHeadersToCurlSList(struct curl_slist*& aSList)
{
    theMap.insert(std::pair<std::string, std::string>("Accept", ""));
    theMap.insert(std::pair<std::string, std::string>("Pragma", ""));
    for (RequestHeaderMap::requestmapiter_t lIter = theMap.begin(); lIter != theMap.end(); ++lIter) {
        std::stringstream s;
        s << (*lIter).first +": " +(*lIter).second;
        aSList = curl_slist_append(aSList, s.str().c_str());
    }

}

} // end namespace
