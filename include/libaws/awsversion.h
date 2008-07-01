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
