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

namespace CSE384 {

class SenderException : public std::exception
{
   public:       
     SenderException(int errnum): errnum_(errnum)
     {
         msgbuf[0] = '\0';
     }

     virtual const char* what() const throw()
     {
         return strerror_portable(msgbuf, PORTABLE_SOCK_ERR_BUF_SIZE-1, errnum_);
     }

     virtual ~SenderException() throw() 
     {
         // std::cerr << "Socket Exception destroyed " << std::endl;
     }
   protected:
      int errnum_;
      char msgbuf[PORTABLE_SOCK_ERR_BUF_SIZE];
};


class SenderConnectException : public SenderException
{
   public:
     SenderConnectException(int errnum):  SenderException(errnum)
     {}
};


class SenderCloseException : public SenderException
{
   public:
     SenderCloseException(int errnum): SenderException(errnum)
     {}
};


class SenderTransmitMessageHeaderException : public SenderException
{
   public:
     SenderTransmitMessageHeaderException(int errnum): SenderException(errnum)
     {}
};


class SenderTransmitMessageDataException : public SenderException
{
   public:
     SenderTransmitMessageDataException(int errnum): SenderException(errnum)
     {}
};

}
#endif /* SENDEREXCEPTIONS_H_ */
