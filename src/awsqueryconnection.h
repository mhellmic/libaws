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

#include <map>

struct curl_slist;

namespace aws {

  class QueryCallBack;

  class AWSQueryConnection : public AWSConnection {
    protected:
      std::string theDefaultHost;
      std::string theVersion;
      std::string theUrl;

      curl_slist* theSList;

      typedef std::map<std::string, std::string> ParameterMap;
      typedef std::pair<std::string, std::string> ParameterPair;
      typedef ParameterMap::iterator ParameterMapIter;
      
    public:
      static std::string QUERY_DATE_FORMAT;

    public:
      AWSQueryConnection ( const std::string& aAccessKeyId, 
                           const std::string& aSecretAccessKey, 
                           const std::string& aHost,
                           const std::string& aVersion);
      virtual ~AWSQueryConnection();

      virtual void makeQueryRequest ( const std::string& action,  
                                      ParameterMap* aParameterMap, 
                                      QueryCallBack* aCallBackWrapper );

      virtual void setCommonParamaters ( ParameterMap* aParameterMap, const std::string& );
      
      // TODO make it const std::string
      std::string getQueryTimestamp();

      static size_t
      dataReceiver ( void *ptr, size_t size, size_t nmemb, void *data );
      
  };

} /* namespace aws */
#endif /* !AWS_AWSQUERYCONNECTION_H */
