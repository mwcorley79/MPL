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
    TEXT = 1,
    REPLY = 2,
    END = 4,
    QUIT = 8,
    FLUSH = 16,
    DEFAULT = 0,
    DISCONNECT = -1,
    STOP_SENDING = -2,
    STRING = -3,
    BINARY = -4
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
  const int CONTENT_SIZE = sizeof(usize);
  const int HEADER_SIZE = TYPE_SIZE + CONTENT_SIZE;

  // help method to approximate Rust's "to_be_bytes" method for usize type
  inline const u8 *to_be_bytes(const usize &sz, usize &result)
  {
    result = hton64_portable(sz);
    return (const u8 *)&result;
  }

  // help method to approximate Rust's "from_be_bytes" method for usize type
  inline usize from_be_bytes(const u8 bytes[], size_t len)
  {
    return ntoh64_portable(*((usize *)bytes));
  }

  class Message
  {
  public:
    Message(size_t sz) : br(sz)
    {
      if (sz < HEADER_SIZE)
        throw std::invalid_argument("message size must greater than equal to the header size");
      // message type automatically inits vec to 0 (MessageType::DEFAULT == 0) by vector ctor init
    }

    // *** copy and move operations not beeded, because the compiler will generate proper semantics ***
    // copy and move construction
    /*
    Message(Message&& msg): br(std::move(msg.br))
    {}

    Message(const Message& msg): br(msg.br)
    {}
   
    // copy and move assignment
    Message&& operator=(Message&& msg)
    {
        this->br =  std::move(msg.br);
    }
    
    Message& operator=(Message& msg)
    {
        this->br =  msg.br;
    }
    */

    u8 operator[](int index) const
    {
      return br[index];
    }

    u8 &operator[](int index)
    {
      // return const_cast<char&>(static_cast<const Message&>(*this)[index]);
      return br[index];
    }

    void init()
    {
      std::fill(br.begin(), br.end(), 0);
    }

    size_t len() const
    {
      return br.size();
    }

    bool is_empty() const
    {
      return (br.size() == 0);
    }

    void set_type(u8 mt)
    {
      br[0] = mt;
    }

    u8 get_type() const
    {
      return br[0];
    }

    /*-------------------------------------------
       Set message content from buff and set
       content size to length of buff
    */
    void set_content_bytes(const u8 buff[], size_t len)
    {
      set_content_size(len);
      set_field(HEADER_SIZE, buff, len);
    }

    // returns a VecSlice
    auto get_content_bytes()
    {
      return get_field(HEADER_SIZE,
                       get_content_size());
    }

    /*-------------------------------------------
      Set message content from str and set
      content size to length of str
    */
    void set_content_str(const std::string &str)
    {
      set_content_size(str.length());
      set_content_bytes((const u8 *)str.c_str(), str.length());
    }

    std::string get_content_str()
    {
      usize sz = get_content_size();
      auto start_it = br.begin() + HEADER_SIZE;
      auto end_it = start_it + sz;

      // 1. -- reference: C++ / Rust string differences:
      // https://hermanradtke.com/2015/05/03/string-vs-str-in-rust-functions.html
      return std::string(start_it, end_it);
    }

    /*-- set message content size --*/
    void set_content_size(usize sz)
    {
      usize result;
      set_field(TYPE_SIZE, to_be_bytes(sz, result), CONTENT_SIZE);
    }

    usize get_content_size()
    {
      // use structured binding to approx. vector "slices" to avoid creating new vecs / arrays
      auto [bytes, len] = get_field(TYPE_SIZE, CONTENT_SIZE);
      return from_be_bytes(bytes, len);

      // dst.clone_from_slice(bytes); // array from byte slice
      // usize::from_be_bytes(dst)    // usize from byte array
    }

    VecSlice get_bytes()
    {
      return {&br[0], br.size()};
    }

    void set_bytes(VecSlice &slice)
    {
      auto [buff, len] = slice;
      for (int i = 0; i < len; ++i)
        br[i] = buff[i];
    }

    std::vector<u8> &get_mut_ref()
    {
      return br;
    }

    const std::vector<u8> &get_ref()
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
          if (i + foldpoint < this->br.size())
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

    static Message create_msg_str_fit(const std::string &content)
    {
      auto msg_size = content.length() + HEADER_SIZE;
      Message msg(msg_size);
      auto cnt_len = content.length();
      msg.set_content_size(cnt_len);
      if (cnt_len > 0)
        msg.set_content_str(content);
      return msg;
    }

    static Message create_msg_bytes_fit(const u8 content[], size_t len)
    {
      auto msg_size = len + HEADER_SIZE;
      Message msg(msg_size);

      msg.set_content_size(len);
      if (len > 0)
        msg.set_content_bytes(content, len);
      return msg;
    }

    static Message create_msg_header_only()
    {
      auto msg = Message(HEADER_SIZE);
      msg.set_content_size(0);
      return msg;
    }

    void set_field(size_t offset, const u8 buff[], size_t len)
    {
      size_t item_index;
      for (size_t i = 0; i < len; ++i)
      {
        if ((item_index = i + offset) < br.size())
          br[item_index] = buff[i];
        else
        {
          return; // if vector would be overrun, just stop the loop and return
        }
      }
    }

    // use structured binding for vector "slices" to avoid creating new vecs / arrays
    // (buffer pointer and len as a tuple)
    VecSlice get_field(size_t offset, size_t size)
    {
      if(size)
        return {&br[offset], size};
      return {nullptr, 0};
    }

    void set_str(usize offset, const std::string &s)
    {
      auto [buff, len] = str_to_bytes(s);
      set_field(offset, buff, len);
    }

    std::string get_str(usize offset, usize size)
    {
      return str_from_bytes(&br[offset], size);
    }

    //  use stuctured to produce the desire effect: reference to the buffer
    // pointer and the length as a tuple
    VecSlice str_to_bytes(const std::string &str)
    {
      return {(u8 *)str.c_str(), str.length()};
    }

    std::string str_from_bytes(const u8 buff[], size_t len)
    {
      return std::string(buff, &buff[len]);
    }

  private:
    std::vector<u8> br;
  };

  inline std::ostream &operator<<(std::ostream &outs, Message &msg)
  {
    for (int i = 0; i < msg.len(); ++i)
      outs << (char)msg[i];
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
