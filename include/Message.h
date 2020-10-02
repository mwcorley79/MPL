//////////////////////////////////////////////////////////////////////////
// cpp_message - message type used to test MPL (cpp_comm)               //
//               intended exercise to port Dr. Fawcett's Rust message   //
//               and achieve Rust Comm and C++ (MPL) Comm iterop        //
//  Dr. Facwett's Rust message type:                                    //
//   -- https://github.com/JimFawcett/RustCommExperiments/blob/masterRustComm_FixedSizeMsg/rust_message/src/lib.rs    //
//                                                                      //
// Mike Corley, https://mwcorley79.github.io/MikeCorley/, 26 Sept 2020  //
//////////////////////////////////////////////////////////////////////////
/*
   Message:
   - fixed size header holding a MessageType attribute
   - stores contents in std::Vec<uint8_t>
*/

#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#include <iostream>
#include <string>
#include <cstring>
#include <memory>
#include <vector>
#include <exception>
#include <iomanip>
#include "Platform.h"

namespace CSE384
{
  using MessageType = enum {
    DEFAULT = 0,
    TEXT = 1,
    REPLY = 2,
    END = 4,
    QUIT = 8,
    DISCONNECT = 8,
    FLUSH = 16,
    STOP_SENDING = 32,
    STRING = 64,
    BINARY = 128
  };

  using usize = uint64_t;
  using u8 = uint8_t;

  // setup for structured binding to define "slices" of vector in terms
  // of buffer (pointer), and length of the slice to avoid explict copy overhead
  // intent is efficient approximated for Rust's borrowed slices (without using C++ 20 ranges yet)
  using VecSlice = std::tuple<const u8 *const, size_t>;

  // Save for ref: (immutability C++)
  // https://www.modernescpp.com/index.php/c-core-guidelines-rules-for-constants-and-immutability

  const int TYPE_SIZE = 1;
  const int TYPE_INDEX = 0;
  const int CONTENT_SIZE = sizeof(usize);
  const int CONTENT_SIZE_INDEX = 1; 
  const int HEADER_SIZE = TYPE_SIZE + CONTENT_SIZE;
  
  inline void to_be_bytes(usize &sz, const usize& val)
  {
    sz = hton64_portable(val);
  }

  inline void from_be_bytes(usize &sz, const usize& val)
  {
    sz = ntoh64_portable(val);
  }

  // helper method to approximate Rust's "to_be_bytes" method for usize type
  inline void to_be_bytes(usize &sz)
  {
    sz = hton64_portable(sz);
  }

  // helper method to approximate Rust's "to_be_bytes" method for usize type
  inline void from_be_bytes(usize &sz)
  {
    sz = ntoh64_portable(sz);
  }
  
  // helper method to approximate Rust's "from_be_bytes" method for usize type
  inline usize from_be_bytes_copy(const usize& sz)
  {
    return ntoh64_portable(sz);
  }

  // helper method to approximate Rust's "from_be_bytes" method for usize type
  inline usize from_be_bytes(const u8 bytes[], size_t len)
  {
    return ntoh64_portable(*((usize *)bytes));
  }

  class Message
  {
    public:
  
    Message(usize sz, u8 mtype=MessageType::DEFAULT): br_len_(sz), br(new u8[br_len_])
    {
      if(br_len_ < HEADER_SIZE)
         throw std::invalid_argument("message size must be greater than equal to the header size");

      set_type(mtype);
      set_content_len(0);
    }

    Message(usize sz, 
            const u8 content_buf[], 
            usize content_len,  
            u8 mtype): br_len_(sz), 
                       br(new u8[br_len_])
    {
      if(br_len_ < HEADER_SIZE)
         throw std::invalid_argument("message size must be greater than equal to the header size");    
      set_type(mtype);
      set_content_bytes(content_buf, content_len);
    }

    Message(usize sz,
            const std::string& str,
            u8 mtype):   Message(sz, (u8*) str.c_str(),  str.length(), mtype)
    {}

    /* void resize(usize sz)
    {
      
      if(br_len_ < HEADER_SIZE)
         throw std::invalid_argument("message size must be greater than equal to the header size");

      u8* temp = new u8[sz];
    }
    */

    // constructs a default (empty content) message with min HEADER_SIZE
    Message(MessageType mtype=MessageType::DEFAULT): Message(HEADER_SIZE, mtype) 
    {
    }

    Message(u8 header[]) 
    {
       // determine the content size
       br_len_ =  HEADER_SIZE + from_be_bytes_copy(*((usize*) (header + TYPE_SIZE)));
       br = new u8[br_len_];
       
       // copy in the header (as passed in)
       std::memcpy(br, header, HEADER_SIZE);
       //std::memset(br + HEADER_SIZE, 0,  max_content_len());
    }

    ~Message() { delete[] br; }

    // move construction
    Message(Message &&msg)
    {
      br = msg.br;
      br_len_ = msg.br_len_;

      msg.br = nullptr;
      msg.br_len_ = 0;
    }

    // copy constructor
    Message(const Message &msg) : br_len_(msg.br_len_), br(new u8[br_len_])
    {
      std::memcpy(br, msg.br, br_len_);
    }

    // move assignment
    Message &operator=(Message &&msg)
    {
      if (&msg != this)
      {
        delete[] br;
        br = msg.br;
        br_len_ = msg.br_len_;
        msg.br = nullptr;
        msg.br_len_ = 0;
      }
      return *this;
    }

    // copy assignment
    Message &operator=(const Message &msg)
    {
      if (&msg != this)
      {
        delete[] br;
        br_len_ = msg.br_len_;
        br = new u8[br_len_];
        std::memcpy(br, msg.br, br_len_ );
      }
      return *this;
    }

    u8 operator[](int index) const
    {
      return br[index];
    }

    u8 &operator[](int index)
    {
      // return const_cast<char&>(static_cast<const Message&>(*this)[index]);
      return br[index];
    }

    size_t max_content_len() const
    {
      return (raw_len() - HEADER_SIZE);
    }

    usize get_content_len() const
    {
      return from_be_bytes_copy(*((usize*) (br + TYPE_SIZE)));
    }

    void set_content_len(const usize &sz)
    {
       to_be_bytes(*((usize*) (br + TYPE_SIZE)), sz);
    }

    size_t raw_len() const
    {
      return br_len_;
    }

    bool is_empty() const
    {
      return (br_len_ == 0);
    }

    void set_type(u8 mt)
    {
      br[TYPE_INDEX] = mt;
    }

    unsigned get_type() const
    {
      return br[TYPE_INDEX];
    }

    /*-------------------------------------------
       Set message content from buff and set
       content size to length of buff
    */
    void set_content_bytes(const u8 buff[], size_t len)
    {
      usize clen = max_content_len();
      if(len < clen)
      {
          std::memcpy(br+HEADER_SIZE, buff, len);
          set_content_len((usize)len);
      }
       else
       {
          std::memcpy(br+HEADER_SIZE, buff, clen );
          set_content_len((usize)clen);
       }  
    }

    size_t get_header_len() const
    {
        return HEADER_SIZE;
    }

    u8* get_content_bytes() const
    {
      return (br+HEADER_SIZE);
    }

    void set_content_str(const std::string &str)
    {
       set_content_bytes( (u8*) str.c_str(), str.length() );
    }

    std::string get_content_str()
    {
      // https://hermanradtke.com/2015/05/03/string-vs-str-in-rust-functions.html
      return std::string(br+HEADER_SIZE, br+HEADER_SIZE+max_content_len());
    }

    void init_content(u8 val=0)
    {
      std::memset(get_content_bytes(), val,  get_content_len());
    }
  
    u8* get_raw_ref() const
    {
      return br;
    }

    /*-------------------------------------------
      Display message with folded contents
    */
    void show_message(usize fold)
    {
      usize foldpoint = 0;
      while (true)
      {
        std::cout << "\n ";
        for (int i = 0; i < fold; ++i)
        {
          if (i + foldpoint < raw_len())
          {
            std::cout << std::setw(4) << (int)this->br[i + foldpoint];
          }
          else
          {
            return;
          }
        }
        foldpoint += fold;
      }
    }

    std::string type_display()
    {
      std::string rtn = std::string("UNKNOWN");
      if (br[0] == MessageType::DEFAULT)
        rtn = std::string("DEFAULT");
      else if (br[0] == MessageType::END)
        rtn = std::string("END");
      else if (br[0] == MessageType::QUIT)
        rtn = std::string("QUIT");
      else if (br[0] == MessageType::REPLY)
        rtn = std::string("REPLY");
      else if (br[0] == MessageType::TEXT)
        rtn = std::string("TEXT");
      else if (br[0] == MessageType::FLUSH)
        rtn = std::string("FLUSH");

      return rtn;
    }


  private:
    size_t br_len_;
    u8 *br;
  };

  inline std::ostream &operator<<(std::ostream &outs, Message &msg)
  {
    usize clen = msg.get_content_len();
    u8* cbytes = msg.get_content_bytes();
    for (int i = 0; i < clen; ++i)
      outs << (unsigned) cbytes[i];
    return outs;
  }

  inline std::ostream &operator<<(std::ostream &outs, const Message &msg)
  {
    return (outs << const_cast<Message &>(msg));
  }

  // prints a structured binding:  my approximation of slices
  inline std::ostream &operator<<(std::ostream &outs, const VecSlice &slice)
  {
    auto [buff, len] = slice;

    int i;
    outs << "[";
    if (len > 0)
    {
      for (i = 0; i < len - 1; ++i)
        outs << (unsigned)buff[i] << ", " << buff[i];
      outs << (unsigned)buff[i] << "]";
    }
    else
    {
      outs << "]";
    }

    return outs;
  }

} // namespace CSE384
#endif
