#ifndef LIBAWS_EXCEPTION_API_H
#define LIBAWS_EXCEPTION_API_H

#include <string>
#include <exception>

namespace aws {

  class AWSException : public std::exception
  {
    public:
      virtual ~AWSException() throw();
      virtual const char* what() const throw();
  };
  

  class AuthenticationException : public AWSException
  {
    public:
      virtual const char* what() const throw();
  };
  

  class AWSAccessKeyIdMissingException : public AuthenticationException
  {
    public:
      virtual const char* what() const throw();
  };  


  class AWSSecretAccessKeyMissingException : public AuthenticationException
  {
    public:
      virtual const char* what() const throw();
  };  


  class AWSConnectionException : public AWSException
  {
    public:
      AWSConnectionException ();
      
      AWSConnectionException(const std::string& aErrorString);
  
      virtual ~AWSConnectionException() throw();
    
      virtual const char* 
      what() const throw();
      
    protected:
      std::string theErrorString;
  };

  class AWSInitializationException : public AWSException
  {
    public:
      AWSInitializationException(const std::string& aErrorString);
  
      virtual ~AWSInitializationException() throw();
  
      virtual const char*
      what() const throw();
  
    protected:
        std::string theErrorString;
  };
  
} /* namespace aws */
#endif
