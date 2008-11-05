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
#include <ostream>
#include <cassert>
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <libaws/awstime.h>

namespace aws {
  Time::Time() : theTime(0) {}
  Time::Time ( time_t aTime) : theTime(aTime) {}
  
  Time::Time ( const char* const aDateTime, const char* const aFormat) {
    setUp(std::string(aDateTime), std::string(aFormat));
  }
  
  Time::Time ( const std::string& aDateTime, const std::string& aFormat) {
    setUp(aDateTime, aFormat);
  }
  
  Time::Time ( int aTimeZoneDiff ) {
    theTime = time ( 0 );
    theTime -= aTimeZoneDiff * 3600;
  }
  
  Time::Time ( const Time& aTime) : theTime(aTime.theTime) {}
  
  void Time::setUp(const std::string& aDateTime, const std::string& aFormat) {
    struct tm aTm;
    memset(&aTm, 0, sizeof(aTm));
#ifndef NDEBUG
    char* lParseResult = strptime(aDateTime.c_str(), aFormat.c_str(), &aTm);
    assert(lParseResult);
#else
    strptime(aDateTime.c_str(), aFormat.c_str(), &aTm);
#endif
    theTime = mktime(&aTm);
    
    std::string::size_type aLoc = aDateTime.find( "GMT", 0 );
    if( aLoc != std::string::npos && (aLoc + 3 < aDateTime.size()) ) {
      std::string lSub(aDateTime.substr(aLoc+3));
      int zoneDiff = atoi(lSub.c_str());
      theTime -= zoneDiff * 3600;
    }
    
    aLoc = aDateTime.find( "UTC", 0 );
    if( aLoc != std::string::npos && (aLoc + 3 < aDateTime.size()) ) {
      std::string lSub(aDateTime.substr(aLoc+3));
      int zoneDiff = atoi(lSub.c_str());
      theTime -= zoneDiff * 3600;
    }
  }
  
  Time& Time::operator+=( time_t aTime ) {
    theTime += aTime;
    return *this;
  }
  
  time_t Time::operator-(const Time& aTime) const {
    return (theTime - aTime.theTime);
  }
  
  Time& Time::operator-=( time_t aTime) {
    theTime -= aTime;
    return *this;
  }
  
  bool Time::operator<(const Time& aTime) const {
    return theTime < aTime.theTime;
  }
  
  bool Time::operator<=(const Time& aTime) const {
    return theTime <= aTime.theTime;
  }
  
  bool Time::operator==(const Time& aTime) const {
    return theTime == aTime.theTime;
  }
  
  bool Time::operator!=(const Time& aTime) const {
    return theTime != aTime.theTime;
  }
  
  bool Time::operator>(const Time& aTime) const {
    return theTime > aTime.theTime;
  }
  
  bool Time::operator>=(const Time& aTime) const {
    return theTime >= aTime.theTime;
  }

  struct tm Time::getStruct() const {
    struct tm lTm = *localtime ( &theTime );
    return lTm;
  }
  
} /* namespace aws */

std::ostream & operator << ( std::ostream & aOStream, const aws::Time & aTime ) {
  struct tm lTm = aTime.getStruct();
  return aOStream << asctime(&lTm);
}
