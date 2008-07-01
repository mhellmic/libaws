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
    s << (aIsSecure ? "https://": "http://") << aServer  << ":" << aPort
      << getPathBase(aBucketName, aKey) << Canonizer::convertPathArgs(aPathArgs);
    return s.str();
}

RegularCallingFormat::~RegularCallingFormat()
{
}

} /* namespace aws */
