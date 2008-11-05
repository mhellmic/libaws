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
#include <algorithm>

struct curl_slist;

namespace aws {

  class QueryCallBack;
  class QueryResponse;

  class AWSQueryConnection : public AWSConnection {
    private:
      DECLARE_LOGGER;
      
    protected:
      std::string theDefaultHost;
      std::string theVersion;

      curl_slist* theSList;

      struct ltstr
      {
        bool operator()(std::string s1, std::string s2) const
        {
          std::transform(s1.begin(), s1.end(), s1.begin(), tolower);
          std::transform(s2.begin(), s2.end(), s2.begin(), tolower);

          return s1.compare(s2) < 0;
        }
      };

      typedef std::map<std::string, std::string, ltstr> ParameterMap;
      typedef std::pair<std::string, std::string> ParameterPair;
      typedef ParameterMap::iterator ParameterMapIter;

    public:
      static std::string QUERY_DATE_FORMAT;

    public:
      AWSQueryConnection ( const std::string& aAccessKeyId,
                           const std::string& aSecretAccessKey,
                           const std::string& aHost,
                           const std::string& aVersion,
                           int aPort = 80,
                           bool aIsSecure = true);
      virtual ~AWSQueryConnection();

      virtual void makeQueryRequest ( const std::string& aUrl,
                                      const std::string& aAction,
                                      ParameterMap* aParameterMap,
                                      QueryCallBack* aCallBackWrapper );

      virtual void makeQueryRequest ( const std::string& aAction,
                                      ParameterMap* aParameterMap,
                                      QueryCallBack* aCallBackWrapper );

      virtual void makeQueryRequestOnResource ( const std::string& aResource,
                                      const std::string& aAction,
                                      ParameterMap* aParameterMap,
                                      QueryCallBack* aCallBackWrapper );

      virtual void setCommonParamaters ( ParameterMap* aParameterMap, const std::string& );

      // TODO make it const std::string
      std::string getQueryTimestamp();

      static size_t
      dataReceiver ( void *ptr, size_t size, size_t nmemb, void *data );
      
      virtual void setCommons(QueryCallBack& aHandler, QueryResponse* aResponse);

  };

} /* namespace aws */
#endif /* !AWS_AWSQUERYCONNECTION_H */
