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
#ifndef AWS_SDBRESPONSE_API_H
#define AWS_SDBRESPONSE_API_H

#include <vector>
#include <map>
#include <string>
#include <libaws/common.h>

namespace aws {

  // forward declaration of internal response classes
  namespace sdb {
    class CreateDomainResponse;
  } /* namespace sqs */

  template <class T>
  class SDBResponse : public SmartObject
  {
    public:
      virtual ~SDBResponse();

      virtual const std::string&
      getRequestId() const;

      virtual const std::string&
      getBoxUsage() const;

    protected:
      T* theSDBResponse;
      SDBResponse ( T* );
  };

  class CreateDomainResponse : public SDBResponse<sdb::CreateDomainResponse >
  {
    public:
      virtual ~CreateDomainResponse() {};

    protected:
      friend class SDBConnectionImpl;
      CreateDomainResponse ( sdb::CreateDomainResponse* );
  };

} /* namespace aws */
#endif
