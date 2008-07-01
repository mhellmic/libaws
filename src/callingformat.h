#ifndef AWS_CALLINGFORMAT_H
#define AWS_CALLINGFORMAT_H

#include <map>
#include "common.h"

namespace aws { 
	
  class RegularCallingFormat;
  typedef class std::map<std::string, std::string> PathArgs_t;

  class CallingFormat {

    public:    
      virtual ~CallingFormat();
      virtual std::string getEndpoint(std::string aServer, int aPort, std::string aBucketName) = 0;
      virtual std::string getPathBase(std::string aBucketName, std::string aKey) = 0;
      virtual std::string getUrl(bool aIsSecure, std::string aServer, 
                                 int aPort, std::string aBucketName, 
                                 std::string aKey, PathArgs_t* aPathArgs) = 0;

    public:
      static RegularCallingFormat*    getRegularCallingFormat();
  };

  class RegularCallingFormat : public CallingFormat {
    public:
      virtual ~RegularCallingFormat();
      virtual std::string getEndpoint(std::string aServer, int aPort, std::string aBucketName);
      virtual std::string getPathBase(std::string aBucketName, std::string aKey);
      virtual std::string getUrl(bool aIsSecure, std::string aServer, 
                                 int aPort, std::string aBucketName, 
                                 std::string aKey, PathArgs_t* aPathArgs);

    private:
      bool isBucketSpecified(std::string aBucketName);
  };

} /* namespace aws */


#endif 

