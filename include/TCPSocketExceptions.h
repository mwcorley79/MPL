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
#include "Platform.h"


class SocketException : public std::exception
{
public:
    SocketException(int errnum) : errnum_(errnum)
    {
        msgbuf[0] = '\0';
    }

    virtual const char* what() const throw()
    {
        return strerror_portable(msgbuf, PORTABLE_SOCK_ERR_BUF_SIZE-1, errnum_);
    }

    virtual ~SocketException() throw()
    {
        // std::cerr << "Socket Exception destroyed " << std::endl;
    }
protected:
    int errnum_;
    char msgbuf[PORTABLE_SOCK_ERR_BUF_SIZE];
};

  class SocketCreateException : public SocketException
  {
    public:
      SocketCreateException(int errnum): SocketException(errnum)
      {}
  };

    class SocketOptionsException : public SocketException
    {
        public:
          SocketOptionsException(int errnum): SocketException(errnum)
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
