#ifndef AWS_AWSCONNECTIONFACTORYIMPL_H
#define AWS_AWSCONNECTIONFACTORYIMPL_H

#include "common.h"
#include <libaws/awsconnectionfactory.h>

namespace aws {

  class AWSConnectionFactoryImpl : public AWSConnectionFactory {

    public:
      AWSConnectionFactoryImpl();
      ~AWSConnectionFactoryImpl();

      S3ConnectionPtr
      createS3Connection(const std::string& aAccessKeyId,  const std::string& aSecretAccessKey); 

      void 
      shutdown();

      std::string
      getVersion();

      // initialization called during static initialization
      // called from getInstance on the first call or after shutdown has been called
      void init();

      // remember whether we have to (re)initialize
      bool theIsInitialized;

      // set to true if initialization fails
      bool theInitializationFailed;

      // error messages reported during initializing libcurl
      std::string theInitializationErrorMessage;

  }; /* class AWSConnectionFactoryImpl */

} /* namespace aws */



#endif
