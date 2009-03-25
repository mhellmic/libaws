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
#include "common.h"

// for initialization and destruction
#include <libxml/parser.h>
#include <libxml/xmlversion.h>
#include <curl/curl.h>

#include <libaws/exception.h>
#include <libaws/awsversion.h>

#include "api/awsconnectionfactoryimpl.h"
#include "api/s3connectionimpl.h"
#include "api/sqsconnectionimpl.h"
#include "api/sdbconnectionimpl.h"

namespace aws {

  AWSConnectionFactoryImpl::AWSConnectionFactoryImpl()
      : theIsInitialized ( false ),
      theInitializationFailed ( false )
  { }

  void
  AWSConnectionFactoryImpl::checkParameters ( const std::string& aAccessKeyId,
                                              const std::string& aSecretAccessKey ) const
  {
    if ( theInitializationFailed )
      throw AWSInitializationException ( theInitializationErrorMessage );

    if ( aAccessKeyId.size() == 0 )
      throw AWSAccessKeyIdMissingException();

    if ( aSecretAccessKey.size() == 0 )
      throw AWSSecretAccessKeyMissingException();
  }

  S3ConnectionPtr
  AWSConnectionFactoryImpl::createS3Connection ( const std::string& aAccessKeyId,
      const std::string& aSecretAccessKey,
      const std::string& aCustomHost ) const
  {

    checkParameters ( aAccessKeyId, aSecretAccessKey );

    return new S3ConnectionImpl ( aAccessKeyId, aSecretAccessKey, aCustomHost );
  }

  SQSConnectionPtr
  AWSConnectionFactoryImpl::createSQSConnection ( const std::string &aAccessKeyId,
      const std::string &aSecretAccessKey,
      const std::string& aCustomHost ) const
  {
    checkParameters ( aAccessKeyId, aSecretAccessKey );

    return new SQSConnectionImpl ( aAccessKeyId, aSecretAccessKey, aCustomHost );
  }

  SQSConnectionPtr
  AWSConnectionFactoryImpl::createSQSConnection ( const std::string &aAccessKeyId,
      const std::string &aSecretAccessKey, const std::string& aCustomHost, int aPort, bool aIsSecure) const
  {
    checkParameters ( aAccessKeyId, aSecretAccessKey );

    return new SQSConnectionImpl ( aAccessKeyId, aSecretAccessKey, aCustomHost, aPort, aIsSecure );
  }

  SDBConnectionPtr
  AWSConnectionFactoryImpl::createSDBConnection ( const std::string &aAccessKeyId,
      const std::string &aSecretAccessKey,
      const std::string& aCustomHost ) const
  {
    checkParameters ( aAccessKeyId, aSecretAccessKey );

    return new SDBConnectionImpl ( aAccessKeyId, aSecretAccessKey, aCustomHost );
  }

  AWSConnectionFactoryImpl::~AWSConnectionFactoryImpl()
  {
    if ( theIsInitialized )
      shutdown();
  }

  void
  AWSConnectionFactoryImpl::shutdown()
  {
    if ( !theInitializationFailed ) {
      xmlCleanupParser();
      curl_global_cleanup();
    }
    theIsInitialized = false;
  }

  std::string
  AWSConnectionFactoryImpl::getVersion()
  {
    return AWSVersion::getAWSVersion();
  }

  void
  AWSConnectionFactoryImpl::init()
  {
    // initialize the curl library
    // this call is not thread safe
    // fortunately, we call it only once when initializing
    // libaws statically.
    CURLcode lCurlCode = curl_global_init ( CURL_GLOBAL_ALL );

    if ( lCurlCode ) {
      const char* lCurlError = curl_easy_strerror ( lCurlCode );
      const std::string lCurlErrorStr ( lCurlError );
      throw AWSConnectionException ( lCurlErrorStr );
    }

    // initialize the libxml2 library and perform version check
    LIBXML_TEST_VERSION

    theIsInitialized = true;
  }


} /* namespace aws */
