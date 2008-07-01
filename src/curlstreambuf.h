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
#ifndef AWS_CURL_STREAMBUF_H
#define AWS_CURL_STREAMBUF_H

#include <streambuf>

typedef void CURL;
typedef void CURLM;

namespace aws { namespace s3 {

class CurlStreamBuffer : public std::streambuf
{
public:
  CurlStreamBuffer(CURL* aEasyHandle);
  virtual ~CurlStreamBuffer();

  virtual int 
  overflow(int c);

  virtual int 
  underflow();

  virtual int 
  multi_perform();

protected:
  CURLM* theMultiHandle;
  CURL*  theEasyHandle;

  // callback called by curl
  static size_t
  write_callback(char *buffer, size_t size, size_t nitems, void *userp);

  static const int INITIAL_BUFFER_SIZE = 1024;
};

} /* namespace s3 */
} /* namespace aws */
#endif
