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
  using MessageType = enum { 
                             DEFAULT = 0,
                             DISCONNECT = -1,
                             STOP_SENDING = -2,
                             STRING = -3, 
                             BINARY = -4};

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

    MSGHEADER(uint32_t l, int32_t t)
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
    Message(const std::string &str, int type);  
    Message(const char *data, size_t length, int type);
    Message(const MSGHEADER &mhdr);
    Message(size_t fixed_size, const char* data, size_t length, int type);
    Message(size_t fixed_size, size_t length, int type);
    Message &operator=(const Message &msg); 
    Message &operator=(Message &&msg);
    
    Message(const Message &msg);
    Message(Message &&msg);

    //static MessagePtr AllocateEmptyFixedSizeMessage(int msg_size);
    static MessagePtr CreateMessage(const MSGHEADER &mhdr);
    static MessagePtr CreateMessage(const char *data, size_t length, int type);
    static MessagePtr CreateMessage(const std::string &str, int type); 

    static MessagePtr CreateFixedSizeMessage(size_t msg_size, const char *data, size_t length, int type);
    static MessagePtr CreateFixedSizeMessage(size_t msg_size, const std::string &str, int type);
    static MessagePtr CreateEmptyFixedSizeMessage(size_t msg_size);

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

    size_t RawMsgLength() const;
    char* GetRawMsg() const;
    
  private:  
    size_t raw_len_;
    char *raw_msg_; 
    MSGHEADER shadow_hdr;
  };

  inline MessagePtr Message::CreateFixedSizeMessage(size_t msg_size, const char *data, size_t length, int type)
  {
     return MessagePtr (new Message(msg_size, data, length, type));
  }

  inline MessagePtr Message::CreateFixedSizeMessage(size_t msg_size, const std::string &str, int type)
  {
     return CreateFixedSizeMessage(msg_size, str.c_str(), str.length(), type); 
  }

  inline MessagePtr Message::CreateEmptyFixedSizeMessage(size_t msg_size)
  {
      return MessagePtr(new Message(msg_size, msg_size,DEFAULT));
  }

  inline MessagePtr Message::CreateMessage(const MSGHEADER &mhdr)
  {
     return MessagePtr(new Message(mhdr));
  }

  inline MessagePtr Message::CreateMessage(const std::string &str, int type)
  {
     return MessagePtr(new Message(str, type));
  } 

  inline MessagePtr Message::CreateMessage(const char *data, size_t length, int type)
  {
     return MessagePtr(new Message(data, length, type));
  }

  inline Message::Message(size_t fixed_size, size_t length, int type): raw_len_(sizeof(MSGHEADER) + fixed_size),
                                                                 raw_msg_(new char[raw_len_])
  {
    // use placement new to init MSG_HDR in raw_msg_ memory space
    // shadow hdr is a local copy of the hdr, becuase MSGHDR must be logically immutable
    shadow_hdr =  *(new (raw_msg_) MSGHEADER( (uint32_t) length, type));
    std::memset((raw_msg_ + sizeof(MSGHEADER)), 0, fixed_size);
  }

  inline Message::Message(size_t fixed_size, const char* data, size_t length, int type): raw_len_(sizeof(MSGHEADER) + fixed_size),
                                                                                   raw_msg_(new char[raw_len_])
  {
    // use placement new to init MSG_HDR in raw_msg_ memory space
     // shadow hdr is a local copy of the hdr, becuase MSGHDR must be logically immutable
    shadow_hdr =  *(new (raw_msg_) MSGHEADER( (uint32_t) length, type));
    std::memset((raw_msg_ + sizeof(MSGHEADER)), 0, fixed_size);
    std::memcpy((raw_msg_ + sizeof(MSGHEADER)), data, length);
  }

  inline Message::Message(const char *data, size_t length, int type) : raw_len_(sizeof(MSGHEADER) + (int) length),
                                                                             raw_msg_(new char[raw_len_])
  {
    // use placement new to instantiate MSG_HDR in raw_msg_ memory space
     // shadow hdr is a local copy of the hdr, becuase MSGHDR must be logically immutable
    shadow_hdr =  *(new (raw_msg_) MSGHEADER((uint32_t)length, type));

    // copy the message into the data portion of the raw message
    std::memcpy((raw_msg_ + sizeof(MSGHEADER)), data, length);
  }

  inline Message::Message(const std::string &str, int type) : Message(str.c_str(), str.length(), type)
  {
  }
 
  inline Message::Message(const MSGHEADER &hdr) :  raw_len_(sizeof(MSGHEADER) + hdr.len()),
                                                   raw_msg_(new char[raw_len_])
  {
    // use placement new to instantiate MSG_HDR in raw_msg_ memory space
    // shadow hdr is a local copy of the hdr, becuase MSGHDR must be logically immutable
    shadow_hdr =  *(new (raw_msg_) MSGHEADER(hdr.len(), hdr.type()));
    std::memset((raw_msg_ + sizeof(MSGHEADER)), 0, Length());
  }

  inline size_t Message::RawMsgLength() const
  {
    return raw_len_;
  }

  inline char* Message::GetRawMsg() const
  {
      return raw_msg_;
  }

  inline MessageType Message::GetType() const
  {
    return (MessageType) shadow_hdr.type();
    //return (MessageType)((MSGHEADER *)raw_msg_)->type();
  }

  inline int Message::Length() const
  {
    return shadow_hdr.len();
   // return ((MSGHEADER *)raw_msg_)->len();
  }

  inline MSGHEADER *Message::GetHeader()
  {
    // must return the actual message hdr (not shadow hdr) because socket Send/Receive will perform endianess conversions
    return ((MSGHEADER *)raw_msg_);
  }

  inline char *Message::GetData() const
  {
    return (raw_msg_ + sizeof(MSGHEADER));
  }

  inline Message::Message(Message &&msg) : raw_msg_(msg.raw_msg_)
  {
    raw_len_ = msg.raw_len_;
    msg.raw_msg_ = nullptr;
    msg.raw_len_ = 0;
    shadow_hdr = msg.shadow_hdr;
    msg.shadow_hdr.data = 0;
  }

  inline Message::Message(const Message &msg) : raw_msg_(new char[msg.RawMsgLength()])
  {
    std::memcpy(raw_msg_, msg.raw_msg_, msg.RawMsgLength());
    raw_len_ = msg.raw_len_;
    shadow_hdr = msg.shadow_hdr;
  }

  inline Message::~Message()
  {
    delete[] raw_msg_;
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

  std::ostream &operator<<(std::ostream &outs, Message &msg);
  std::ostream &operator<<(std::ostream &outs, const Message &msg);

} // namespace CSE384
#endif
