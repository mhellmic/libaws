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
#ifndef AWS_ConnectionPool
#define AWS_ConnectionPool

#include <pthread.h>
#include <queue>
#include <libaws/mutex.h>
#include <libaws/aws.h>
#include <libaws/s3connection.h>
#include <libaws/awsconnectionfactory.h>
#include <iostream>

using namespace aws;

namespace aws { 

template <class T>
class ConnectionPool : public std::queue<T>
{

private:

    AWSConnectionFactory* theFactory;
    AWSMutex theConnectionPoolMutex;
    std::string theAccessKeyId;
    std::string theSecretAccessKey;
    unsigned int theSize;

    T createConnection (const std::string& aAccessKeyId,
      const std::string& aSecretAccessKey);

public:

    ConnectionPool(unsigned int size, const std::string& accesskeyid, const std::string& secretaccesskey);

    ~ConnectionPool();

    void release(T connection);

    T getConnection();

};

}//namespace aws


#endif
