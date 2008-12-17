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
#include <libaws/mutex.h>

namespace aws {

  AWSMutex::AWSMutex()
  {
      //(void) pthread_mutexattr_init(theMutexAttr);
      pthread_mutex_init(&theMutex, 0);
  }

  void AWSMutex::lock()
  {
     pthread_mutex_lock(&theMutex);
  }

  void AWSMutex::unlock()
  {
     pthread_mutex_unlock(&theMutex);
  }

} // namespace
