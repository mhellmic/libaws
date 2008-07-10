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
#ifndef AWS_SDB_SDBCONNECTION_H
#define AWS_SDB_SDBCONNECTION_H

#include "common.h"

#include <map>
#include <iostream>

#include "awsqueryconnection.h"

namespace aws {

  namespace sdb {

    class CreateDomainResponse;
    
    class SDBConnection : public AWSQueryConnection
    {
      public:
        static const std::string DEFAULT_VERSION;
        static const std::string DEFAULT_HOST;
        
      
      public:
        SDBConnection(const std::string& aAccessKeyId, 
                      const std::string& aSecretAccessKey);
      
        CreateDomainResponse*
            createDomain ( const std::string &aQueueName );
    };
    
  } /* namespace sdb  */
} /* namespace aws */

#endif        
