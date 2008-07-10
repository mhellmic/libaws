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

#include "sdb/sdbresponse.h"

namespace aws {

  template <class T>
  SDBResponse<T>::SDBResponse ( T* aResponse )
      : theSDBResponse ( aResponse ) {}

  template <class T>
  SDBResponse<T>::~SDBResponse()
  {
    delete theSDBResponse;
  }

  template <class T>
  const std::string&
  SDBResponse<T>::getRequestId() const
  {
    return theSDBResponse->getRequestId();
  }

  template <class T>
  const std::string&
  SDBResponse<T>::getBoxUsage() const
  {
    return theSDBResponse->getBoxUsage();
  }

  /**
   * CreateDomainResponse
   */
  CreateDomainResponse::CreateDomainResponse ( sdb::CreateDomainResponse* r )
      : SDBResponse<sdb::CreateDomainResponse> ( r ) {}


} /* namespace aws */
