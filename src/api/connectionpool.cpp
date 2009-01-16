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
#include <libaws/connectionpool.h>

namespace aws { 

    template<class T>
    ConnectionPool<T>::ConnectionPool(unsigned int size, const std::string& accesskeyid, const std::string& secretaccesskey) :
      theFactory(AWSConnectionFactory::getInstance()),
      theAccessKeyId(accesskeyid),
      theSecretAccessKey(secretaccesskey),
      theSize(size)
    {
      for(unsigned int i=1;i<=size;i++){
         push(createConnection(theAccessKeyId, theSecretAccessKey));
      }
    }

    template<class T>
    ConnectionPool<T>::~ConnectionPool(){
    }


    template<class T>
    void ConnectionPool<T>::release(T connection) { 
      if(theSize>std::queue<T>::size())
      {
         theConnectionPoolMutex.lock(); 
         std::queue<T>::push(connection);
         theConnectionPoolMutex.unlock(); 
      }
    }

    template<class T>
    T ConnectionPool<T>::getConnection() { 
      theConnectionPoolMutex.lock(); 
      if(std::queue<T>::size()>0){

         // there are still connections in the queue, so return one of them
         T connection = std::queue<T>::front(); 
         std::queue<T>::pop(); 
#ifndef NDEBUG
         std::cerr << "[ConnectionPool] still " << (int)std::queue<T>::size() << " connections remaining in the pool" << std::endl;
#endif
         theConnectionPoolMutex.unlock(); 
         return connection; 
       }else{

         // queue is empty -> generate connection dynamically
         theConnectionPoolMutex.unlock(); 
         return createConnection(theAccessKeyId, theSecretAccessKey);
       }
    }

   template<> S3ConnectionPtr 
   ConnectionPool<S3ConnectionPtr>::createConnection ( const std::string& aAccessKeyId,
                                         const std::string& aSecretAccessKey ) {
     return theFactory->createS3Connection(theAccessKeyId, theSecretAccessKey);
   }

   template<> SQSConnectionPtr
   ConnectionPool<SQSConnectionPtr>::createConnection ( const std::string& aAccessKeyId,
                                         const std::string& aSecretAccessKey ) {
    return theFactory->createSQSConnection(theAccessKeyId, theSecretAccessKey);
   }

   template<> SDBConnectionPtr
   ConnectionPool<SDBConnectionPtr>::createConnection ( const std::string& aAccessKeyId,
                                         const std::string& aSecretAccessKey ) {
    return theFactory->createSDBConnection(theAccessKeyId, theSecretAccessKey);
   }

   template class ConnectionPool<S3ConnectionPtr>;
   template class ConnectionPool<SQSConnectionPtr>;

}//namespace aws
