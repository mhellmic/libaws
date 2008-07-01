#include "curlstreambuf.h"

#include <iostream>
#include <curl/curl.h>

namespace aws { namespace s3 {

CurlStreamBuffer::CurlStreamBuffer(CURL* aEasyHandle)
  : std::streambuf(),
    theEasyHandle(aEasyHandle)
{
  theMultiHandle = curl_multi_init();
  curl_easy_setopt(theEasyHandle, CURLOPT_WRITEDATA, this);
  curl_easy_setopt(theEasyHandle, CURLOPT_WRITEFUNCTION, CurlStreamBuffer::write_callback);
  curl_multi_add_handle(theMultiHandle, theEasyHandle);
}

CurlStreamBuffer::~CurlStreamBuffer()
{
  ::free(eback());
  curl_multi_remove_handle(theMultiHandle, theEasyHandle);
  curl_multi_cleanup(theMultiHandle);
}


int
CurlStreamBuffer::multi_perform()
{
  CURLMsg* msg;
  int lMsgsInQueue;
  int lStillRunning = 0;
  bool lDone = false;
  int lError = 0;

  while (!lDone) {
    while (CURLM_CALL_MULTI_PERFORM == curl_multi_perform(theMultiHandle, &lStillRunning))
      ;

    while ((msg = curl_multi_info_read(theMultiHandle, &lMsgsInQueue))) {
      if (msg->msg == CURLMSG_DONE) {
        lError = msg->data.result;
        lDone = true;
      }
    }
  }

  // TODO: return message, too ?
  //if (lError)
  //  std::cout << "error: [" << "]" << std::endl;

  return lError;
}

size_t
CurlStreamBuffer::write_callback(char* buffer, size_t size, size_t nitems, void* userp)
{
  CurlStreamBuffer* sbuffer = static_cast<CurlStreamBuffer*>(userp);
  size_t result = sbuffer->sputn(buffer, size*nitems);
  sbuffer->setg(sbuffer->eback(), sbuffer->gptr(), sbuffer->pptr());
  return result;
}


int
CurlStreamBuffer::overflow(int c)
{
  char* _pptr = pptr();
  char* _gptr = gptr();
  char* _eback = eback();

  int new_size = 2 * (epptr() - _eback);
  if (new_size == 0)
    new_size = INITIAL_BUFFER_SIZE;

  char* new_buffer = (char*)realloc(_eback, new_size);

  if (new_buffer != _eback) {
    _pptr = new_buffer + (_pptr - _eback);
    _gptr = new_buffer + (_gptr - _eback);
    _eback = new_buffer;
  }
  setp(_pptr, new_buffer + new_size);
  sputc(c);
  setg(_eback, _gptr, pptr());

  return 0;
}

int
CurlStreamBuffer::underflow()
{
  // TODO: place a call to multi_perform() ?
  return EOF;
}

} /* namespace s3 */
} /* namespace aws */
