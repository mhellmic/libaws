#include "common.h"

#include <libaws/awsconnectionfactory.h>
#include <libaws/exception.h>

#include "api/awsconnectionfactoryimpl.h"

namespace aws {

  AWSConnectionFactory::~AWSConnectionFactory() {}

  AWSConnectionFactory*
  AWSConnectionFactory::getInstance()
  {
    static AWSConnectionFactoryImpl lInstance;
    if ( ! lInstance.theIsInitialized )
      try {
        lInstance.init();
      } catch (AWSConnectionException& e) {
        lInstance.theInitializationFailed = true;
        lInstance.theInitializationErrorMessage = e.what();
        throw e;
      }
    return &lInstance;
  }

} /* namespace aws */
