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
#ifndef AWS_AWSCONNECTIONFACTORYIMPL_H
#define AWS_AWSCONNECTIONFACTORYIMPL_H

#include "common.h"
#include <libaws/awsconnectionfactory.h>

namespace aws {

  class AWSConnectionFactoryImpl : public AWSConnectionFactory {
    
    friend class AWSConnectionFactory;
    

    protected:
      void
      checkParameters(const std::string& aAccessKeyId,
                      const std::string& aSecretAccessKey) const;

      AWSConnectionFactoryImpl();
      
    public:
      
      virtual ~AWSConnectionFactoryImpl();

      virtual S3ConnectionPtr
      createS3Connection(const std::string& aAccessKeyId,
                         const std::string& aSecretAccessKey,
                         const std::string& aCustomHost) const;

      virtual SQSConnectionPtr
      createSQSConnection(const std::string& aAccessKeyId,
                          const std::string& aSecretAccessKey,
                          const std::string& aCustomHost) const;

      virtual SQSConnectionPtr
      createSQSConnection(const std::string& aAccessKeyId,
                          const std::string& aSecretAccessKey,
                      		const std::string& aHost, int aPort,
                          bool aIsSecure) const;

      virtual SDBConnectionPtr
      createSDBConnection(const std::string& aAccessKeyId,
                          const std::string& aSecretAccessKey,
                          const std::string& aCustomHost) const;

      virtual void
      shutdown();

      virtual std::string
      getVersion();

      // initialization called during static initialization
      // called from getInstance on the first call or after shutdown has been called
      virtual void init();

      // remember whether we have to (re)initialize
      bool theIsInitialized;

      // set to true if initialization fails
      bool theInitializationFailed;

      // error messages reported during initializing libcurl
      std::string theInitializationErrorMessage;

  }; /* class AWSConnectionFactoryImpl */

} /* namespace aws */

#endif
