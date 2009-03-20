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

#include <iostream>
#include <sstream>
#include <string>

#include "callingformat.h"
#include "canonizer.h"

using namespace aws::s3;

namespace aws { 


RegularCallingFormat*
CallingFormat::getRegularCallingFormat() 
{
    static RegularCallingFormat lRegularCallingFormat;
    return &lRegularCallingFormat;
}

CallingFormat::~CallingFormat()
{
    
}

bool
RegularCallingFormat::isBucketSpecified(std::string aBucketName)
{
    return aBucketName.size() != 0;
}

std::string 
RegularCallingFormat::getEndpoint(std::string aServerName, int aPort, std::string /*aBucketName*/) 
{
    std::stringstream s;
    s << aServerName << ":" << aPort;
    return s.str();
}

std::string
RegularCallingFormat::getPathBase(std::string aBucketName, std::string aKey)
{
    return isBucketSpecified(aBucketName) ? "/" + aBucketName + "/" + aKey : "/";
}

std::string
RegularCallingFormat::getUrl(bool aIsSecure, std::string aServer, int aPort, std::string aBucketName, 
                             std::string aKey, PathArgs_t* aPathArgs)
{
    std::stringstream s;
    if (aServer.find_first_of("http://") == std::string::npos &&  aServer.find_first_of("https://") == std::string::npos)
      s << (aIsSecure ? "https://": "http://") << aServer;
    else
      s << aServer;
    if(aPort > 0)
      s << ":" << aPort;
    s << getPathBase(aBucketName, aKey) << Canonizer::convertPathArgs(aPathArgs);
    return s.str();
}

RegularCallingFormat::~RegularCallingFormat()
{
}

} /* namespace aws */
