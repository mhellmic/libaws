#include <libaws/exception.h>

namespace aws {

  AWSException::~AWSException() throw()
  {
  }

  const char* 
  AWSException::what() const throw()
  {
    return "AWSException";
  }

  const char* 
  AuthenticationException::what() const throw()
  {
    return "AuthenticationException";
  }

  const char* 
  AWSAccessKeyIdMissingException::what() const throw()
  {
    return "AWSAccessKeyIdMissingException";
  }

  const char* 
  AWSSecretAccessKeyMissingException::what() const throw()
  {
    return "AWSSecretAccessKeyMissingException";
  }

  AWSConnectionException::AWSConnectionException() {}

  AWSConnectionException::AWSConnectionException(const std::string& aErrorString)
    : theErrorString(aErrorString) {}

  AWSConnectionException::~AWSConnectionException() throw()
  {
  }

  const char* 
  AWSConnectionException::what() const throw()
  {
    return theErrorString.c_str();
  }

  AWSInitializationException::AWSInitializationException(const std::string& aErrorString) 
    : theErrorString(aErrorString) {}

  AWSInitializationException::~AWSInitializationException() throw() {}


  const char* 
  AWSInitializationException::what() const throw()
  {
    return "The initialization of the AWS C++ library failed during the first call of getInstance()";
  }
}
