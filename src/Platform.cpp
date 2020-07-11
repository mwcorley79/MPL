
#include "Platform.h"
 #if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)

  std::atomic<int> SocketSystem::rcount = 0;

  // helper function to get socket errors in a portable way (windows and linux)
  const char* strerror_portable(const char* errbuf, int buflen, int error)
  {
	  // source: https://stackoverflow.com/questions/3400922/how-do-i-retrieve-an-error-string-from-wsagetlasterror
	  FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
		  NULL,                // lpsource
		  error,             // message id
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
		  (LPSTR)errbuf,              // output buffer
		  (size_t)buflen,     // size of msgbuf, bytes
		  NULL);

	  return errbuf;
  }
#endif 