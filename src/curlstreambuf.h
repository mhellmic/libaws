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
