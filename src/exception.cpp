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
