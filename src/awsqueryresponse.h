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


namespace aws
{
  class AWSQueryConnection;

  class QueryResponse
  {
    protected:
      friend class AWSQueryConnection;
      std::string theRequestId;
      double theInTransfer;
      double theOutTransfer;

    public:
      const std::string& getRequestId() const
      {
        return theRequestId;
      }
      
      double getInTransfer() const
      {
        return theInTransfer;
      }
      
      double getOutTransfer() const
      {
        return theOutTransfer;
      }
  };

  class QueryErrorResponse
  {
    protected:
      std::string theErrorCode;
      std::string theErrorMessage;
      std::string theRequestId;
      std::string theUrl;

    public:
      QueryErrorResponse(){};
      QueryErrorResponse(std::string aErrorCode,
                         std::string aErrorMessage,
                         std::string aRequestId,
                         std::string aUrl) :
        theErrorCode(aErrorCode),
        theErrorMessage(aErrorMessage),
        theRequestId(aRequestId),
        theUrl(aUrl){}

        const std::string& getErrorCode() const {return theErrorCode;}
        void setErrorCode(std::string& value){theErrorCode = value;};
        const std::string& getErrorMessage() const {return theErrorMessage;}
        void setErrorMessage(std::string& value){theErrorMessage = value;};
        const std::string& getRequestId() const {return theRequestId;}
        void setRequestId(std::string& value){theRequestId = value;};
        const std::string& getUrl() const {return theUrl;}
        void setUrl(std::string& value){theUrl = value;};
  };

}//namespace aws
#endif
