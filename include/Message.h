#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <iostream>
#include <string>
#include <cstring>
#include <arpa/inet.h>

namespace CSE384
{
    using MessageType = enum {DISCONNECT=-1, STOP_SENDING=-2, STRING=-3};

    // binary message structure: (wire protocol as used by messaging interface)
    struct MSGHEADER
    {
      unsigned int len_:32;  // bit field:  force (4-byte) word alignment
      short int type_:16;    // force (2-byte) word alignment

      void ToHostByteOrder()
      {
        len_ =  ntohl(len_);
        type_ = ntohs(type_);
      }

      void ToNetworkByteOrder()
      {
        len_ =  htonl(len_);
        type_ = htons(type_);
      }
    } __attribute__((__packed__)); // <--need to override compiler memory boundary alignment

    class Message
    {
        public:
          Message(const std::string& str, int type = 0);
          Message(const char* data=0, int length=0, int type=0); 
          Message(struct MSGHEADER& mhdr, char* data=nullptr);
          Message& operator=(const Message& msg);
          Message& operator=(Message&& msg);
          Message(const Message& msg);
          Message(Message&& msg);
          
          //note: don't call this if Message contains binary data
          std::string ToString() const;

          int HeaderSize() const;
          int Length() const;
          MessageType GetType();
          char* GetData() const;
          MSGHEADER* GetHeader();
        private:
          MSGHEADER msg_hdr_;
          char* msg_data_;
    };

    inline MessageType Message::GetType()
    {
        return (MessageType) msg_hdr_.type_;
    }

    inline MSGHEADER* Message::GetHeader()
    {
      return &msg_hdr_;
    }
    
    inline char* Message::GetData() const
    {
      return msg_data_;
    }

    inline int Message::HeaderSize() const
    {
      return sizeof(msg_hdr_);
    }

    inline int Message::Length() const
    {
      return msg_hdr_.len_;
    }

    std::ostream& operator << (std::ostream& outs, Message& msg);
    std::ostream& operator << (std::ostream& outs, const Message& msg);

}
#endif 