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
