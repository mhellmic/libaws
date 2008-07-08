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
#ifndef AWS_AWSQUERYCONNECTION_H
#define AWS_AWSQUERYCONNECTION_H

#include "common.h"
#include "awsconnection.h"


namespace aws {

  class AWSQueryConnection : public AWSConnection {
    protected:
      std::string theDefaultHost;
      std::string theVersion;
      std::string theUrl;
      
    public:
      static std::string QUERY_DATE_FORMAT;

    public:
      AWSQueryConnection ( const std::string& aAccessKeyId, 
                           const std::string& aSecretAccessKey, 
                           const std::string& aHost,
                           const std::string& aVersion);

      virtual void makeQueryRequest ( const std::string& action,  
                                      ParameterMap_t* aParameterMap, 
                                      CallBackWrapper* aCallBackWrapper );

      virtual void setCommonParamaters ( ParameterMap_t* aParameterMap );
      
      std::string getQueryTimestamp();
      
      
  };

} /* namespace aws */
#endif /* !AWS_AWSCONNECTION_H */
