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
#ifndef AWS_UTIL_H
#define AWS_UTIL_H

#include <string.h>

namespace aws {

  void
  trim(std::string& str)
  {
    std::string::size_type pos = str.find_last_not_of(" \n\t");
    if(pos != std::string::npos) {
      str.erase(pos + 1);
      pos = str.find_first_not_of(" \n\t");
      if(pos != std::string::npos) str.erase(0, pos);
    }
    else str.erase(str.begin(), str.end());
  }

} /* namespace aws */

#endif
