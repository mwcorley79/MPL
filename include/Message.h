#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include "Platform.h"


namespace CSE384
{
  class Message;
  using MessagePtr = std::shared_ptr<Message>;
  using MessageType = enum { DISCONNECT = -1,
                             STOP_SENDING = -2,
                             STRING = -3 };

  // binary message structure: (wire protocol as used by messaging interface)
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__) || defined(_WIN64)
#pragma pack(1) // https://docs.microsoft.com/en-us/cpp/preprocessor/pack?view=vs-2019
#endif
  struct MSGHEADER
  {
    //store 16-bit (length), and 16-bit (type) for message header
    unsigned int data : 32; // bit field: force (4-byte) word alignment

    MSGHEADER() : data(0)
    {
    }

    MSGHEADER(int l, int t)
    {
      // store "length" in upper half of the word,
      // store "type" in the lower half   
      data = (l << 16) | (0x0000FFFF & t);
    }

    inline int type() const
    {
      return  (int16_t) (data & 0x0000FFFF);
    }

    inline unsigned int len() const
    {
      return (data & 0xFFFF0000) >> 16;
    }

    inline void ToHostByteOrder()
    {
      data = ntohl(data);
    }

    inline void ToNetorkByteOrder()
    {
      data = htonl(data);
    }

    inline static size_t SIZE() { return sizeof(MSGHEADER); }

#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
  } __attribute__((__packed__)); // <--need to override compiler memory boundary alignment
#else
  };
#endif

  class Message
  {
  public:
    Message(const std::string &str, int type = 0);  
    Message(const char *data = 0, uint16_t length = 0, int16_t type = 0);
    Message(const MSGHEADER &mhdr);
    
    Message &operator=(const Message &msg); 
    Message &operator=(Message &&msg);
    
    Message(const Message &msg);
    Message(Message &&msg);

    
   
    char& operator[](int index);
    const char& operator[](int index) const;

    ~Message();
    
    //note: don't call this if Message contains binary data
    std::string ToString() const;

    int HeaderSize() const;
    int Length() const;
    MessageType GetType() const;
    char *GetData() const;
    MSGHEADER *GetHeader();

    int RawMsgLength() const;
    char* GetRawMsg() const;

  private:
    char *raw_msg_;
  };

  inline Message::~Message()
  {
    delete raw_msg_;
  }

  inline MessageType Message::GetType() const
  {
    return (MessageType)((MSGHEADER *)raw_msg_)->type();
  }

  inline MSGHEADER *Message::GetHeader()
  {
    return ((MSGHEADER *)raw_msg_);
  }

  inline char *Message::GetData() const
  {
    return (raw_msg_ + HeaderSize());
  }

  inline char& Message::operator[](int index)
  {
    // return const_cast<char&>(static_cast<const Message&>(*this)[index]);
    if(index < 0 || Length() <= index  )
       throw std::invalid_argument("index out of bounds in operator[])");
    return GetData()[index];
  }

  inline const char& Message::operator[](int index) const
  {
    if(index < 0 || Length() <= index  )
       throw std::invalid_argument("index out of bounds in operator[])");
    return GetData()[index];
  }

  inline int Message::HeaderSize() const
  {
    return sizeof(MSGHEADER);
  }

  inline int Message::Length() const
  {
    return ((MSGHEADER *)raw_msg_)->len();
  }

  inline int Message::RawMsgLength() const
  {
    return (Length() + HeaderSize());
  }

  inline char* Message::GetRawMsg() const
  {
      return raw_msg_;
  }

  std::ostream &operator<<(std::ostream &outs, Message &msg);
  std::ostream &operator<<(std::ostream &outs, const Message &msg);

} // namespace CSE384
#endif
