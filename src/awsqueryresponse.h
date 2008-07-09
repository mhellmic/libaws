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
#ifndef AWS_AWSQUERYRESPONSE_H
#define AWS_AWSQUERYRESPONSE_H

#include <libxml/parser.h>


namespace aws
{

  class QueryResponse 
  {   
    protected:
      std::string theRequestId;
      
    public: 
      std::string getRequestId() {return theRequestId;}
  };
  
  class QueryErrorResponse
  {
    protected:
      std::string theErrorCode;
      std::string theErrorMessage;
      std::string theRequestId;
      std::string theUrl;
      
    public:
      QueryErrorResponse(std::string aErrorCode, 
                         std::string aErrorMessage, 
                         std::string aRequestId,
                         std::string aUrl) : 
        theErrorCode(aErrorCode), 
        theErrorMessage(aErrorMessage), 
        theRequestId(aRequestId),
        theUrl(aUrl){} 
             
      std::string getErrorCode() {return theErrorCode;}
      std::string getErrorMessage() {return theErrorMessage;}
      std::string getRequestId() {return theRequestId;}
      std::string getUrl() {return theUrl;}
  };
  
}//namespace aws
#endif
