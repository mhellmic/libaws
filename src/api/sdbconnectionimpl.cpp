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
#include <libaws/sdbresponse.h>

#include "sdb/sdbconnection.h"
#include "api/sdbconnectionimpl.h"

namespace aws {

  SDBConnectionImpl::SDBConnectionImpl ( const std::string& aAccessKeyId,
                                         const std::string& aSecretAccessKey )
  {
    theConnection = new sdb::SDBConnection ( aAccessKeyId, aSecretAccessKey );
  }

  SDBConnectionImpl::~SDBConnectionImpl()
  {
    delete theConnection;
  }

  CreateDomainResponsePtr 
  SDBConnectionImpl::createDomain ( const std::string &aDomainName )
  {
    return new CreateDomainResponse ( theConnection->createDomain ( aDomainName ) );
  }

}//namespace aws
