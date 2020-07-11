////////////////////////////////////////////////////////////////////////////////
// SenderExceptions.h - Defines a variety of exceptions thrown by the Sender  //
// Language:    standard C++ 11 (g++ 7.4.x)                                   //
// Platform:    Dell Precision m7720, MINT 19.3 (Linux) 64-bit                //
// Application: CSE 384, Project 2 Helpers                                    //
// Author:      Mike Corley, Syracuse University, mwcorley@syr.edu            //
////////////////////////////////////////////////////////////////////////////////


#ifndef SENDEREXCEPTIONS_H_
#define SENDEREXCEPTIONS_H_

#include <cstring>
#include <exception>
#include <errno.h>
#include <locale.h>

#include "Platform.h"

// for strerror_s on Linux: source: https://en.cppreference.com/w/c/string/byte/strerror
#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
   #ifndef __STDC_WANT_LIB_EXT1__
     #define __STDC_WANT_LIB_EXT1__
   #endif 
#endif


namespace CSE384 {


class SenderException : public std::exception
{
   public:
#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
       SenderException() : errnum_(errno)
#else
       SenderException() : errnum_(WSAGetLastError())
#endif
       {
           msgbuf[0] = '\0';
       }
       
     SenderException(int errnum): errnum_(errnum)
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

     ~SenderException() throw() {}
   protected:
      int errnum_;
      char msgbuf[512];
};


class SenderConnectException : public SenderException
{
   public:
     SenderConnectException()
     {}
};


class SenderCloseException : public SenderException
{
   public:
     SenderCloseException()
     {}
};


class SenderTransmitMessageHeaderException : public SenderException
{
   public:
     SenderTransmitMessageHeaderException()
     {}
};


class SenderTransmitMessageDataException : public SenderException
{
   public:
     SenderTransmitMessageDataException()
     {}
};

}
#endif /* SENDEREXCEPTIONS_H_ */
