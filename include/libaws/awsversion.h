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
#ifndef LIBAWS_VERSION_API_H
#define LIBAWS_VERSION_API_H

#include <string>

namespace aws {

  /*! \brief AWSVersion provides version information about
   *         this version of the AWS C++ library (libaws).
   */
  class AWSVersion
  {
  private:
    static int theMajorVersion;
    static int theMinorVersion;
    static int thePatchVersion;

  public:
    //! AWS C++ library major version
    static int
    getAWSMajorVersion();

    //! AWS C++ library minor version
    static int
    getAWSMinorVersion();


    //! AWS C++ library patch version
    static int
    getAWSPatchVersion();


    /*! \brief AWS C++ library version
     *
     * Provide AWS C++ library version information.
     * @return the version information as string of the form major.minor.patch
     */
    static std::string
    getAWSVersion();

  };

} /* end namespace aws */

#endif
