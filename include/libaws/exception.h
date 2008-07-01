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
