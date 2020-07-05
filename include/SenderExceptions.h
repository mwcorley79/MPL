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

#include "Platform.h"


namespace CSE384 {


class SenderException : public std::exception
{
   public:

     SenderException(int errnum): errnum_(errnum)
     {}

     virtual const char* what() const throw()
     {
    	 return strerror(errnum_);
     }

     ~SenderException() throw() {}
   protected:
      int errnum_;
};


class SenderConnectException : public SenderException
{
   public:
     SenderConnectException(): SenderException(errno)
     {}
};


class SenderCloseException : public SenderException
{
   public:
     SenderCloseException(): SenderException(errno)
     {}
};


class SenderTransmitMessageHeaderException : public SenderException
{
   public:
     SenderTransmitMessageHeaderException(): SenderException(errno)
     {}
};


class SenderTransmitMessageDataException : public SenderException
{
   public:
     SenderTransmitMessageDataException(): SenderException(errno)
     {}
};





}
#endif /* SENDEREXCEPTIONS_H_ */
