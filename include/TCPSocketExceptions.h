/*
 * TCPSocketExceptions.h
 *
 *  Created on: Jul 4, 2020
 *      Author: mwcorley79
 */

#ifndef TCPSOCKETEXCEPTIONS_H_
#define TCPSOCKETEXCEPTIONS_H_

#include <cstring>
#include <exception>
#include<string>
#include <cstdio>
#include <errno.h>
#include <locale.h>

 // for strerror_s on Linux: source: https://en.cppreference.com/w/c/string/byte/strerror
#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
   #ifndef __STDC_WANT_LIB_EXT1__
      #define __STDC_WANT_LIB_EXT1__
   #endif 
#endif


class SocketException : public std::exception
{
public:
#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
    SocketException() : errnum_(errno)
#else
    SocketException() : errnum_(WSAGetLastError())
#endif
    {
        msgbuf[0] = '\0';
    }

    SocketException(int errnum) : errnum_(errnum)
    {
        msgbuf[0] = '\0';
    }

    virtual const char* what() const throw()
    {
#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
#ifdef __STDC_LIB_EXT1__
        strerror_s(msgbuf, 511, errno);
        return msgbuf;
#endif
#else
        // source: https://stackoverflow.com/questions/3400922/how-do-i-retrieve-an-error-string-from-wsagetlasterror
        FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,   // flags
            NULL,                // lpsource
            errnum_,             // message id
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),    // languageid
            (LPSTR)msgbuf,              // output buffer
            sizeof(msgbuf),     // size of msgbuf, bytes
            NULL);

        return msgbuf;
#endif              
    }

    virtual ~SocketException() throw()
    {}
protected:
    int errnum_;
    char msgbuf[512];
};

  class SocketCreateException : public SocketException
  {
    public:
      SocketCreateException()
      {}
  };

    class SocketOptionsException : public SocketException
    {
        public:
          SocketOptionsException()
          {}
    };

    class GetAddrInfoException : public SocketException
    {
       public:
         GetAddrInfoException(int errnum): SocketException(errnum)
         {}
         virtual const char* what() const throw()
         {
        	 return gai_strerror(errnum_);
         }
    };


#endif /* TCPSOCKETEXCEPTIONS_H_ */
