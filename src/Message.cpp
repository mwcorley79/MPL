#ifdef TEST_MESSAGE
#include "Message.h"
#include <iostream>
#include <assert.h>
#include <string>
using namespace CSE384;

int main()
{
    std::cout << "Rust compatible message class" << std::endl;
    std::cout << "Test constructors" << std::endl;
    try
    {
       Message msg(8, MessageType::DEFAULT);
    }
    catch(const std::exception& e)
    {
      std::cerr << e.what() << '\n';
    }

    std::cout << "test zero content len constructor" << std::endl;
    Message emptyMsg(MessageType::TEXT);
    std::cout << "\traw len         : " << emptyMsg.raw_len() << std::endl;
    std::cout << "\tmax content len : " << emptyMsg.max_content_len() << std::endl;
    std::cout << "\tcontent len     : " << emptyMsg.get_content_len() << std::endl;
    std::cout << "\tmessage type    : " << emptyMsg.get_type() << std::endl;
    std::cout << "\tshow message    : "; emptyMsg.show_message(8);
    std::cout << "\tmessage content : " << emptyMsg << std::endl; 
 
    std::cout << "test standard constructor" << std::endl;
    Message msg(100, MessageType::DEFAULT);
    std::cout << "\traw len         : " << msg.raw_len() << std::endl;
    std::cout << "\tmax content len : " << msg.max_content_len() << std::endl;
    std::cout << "\tcontent len     : " << msg.get_content_len() << std::endl;
    std::cout << "\tmessage type    : " << msg.get_type() << std::endl;
    std::cout << "\tshow message    : ";   msg.show_message(8);
    std::cout << "\tmessage content : " << msg << std::endl; 
 
  

    std::cout << "test set_content_bytes method" << std::endl;
    u8 buf[]={1,2,3,4};
    msg.set_content_bytes(buf, 4);
    std::cout << "\traw len         : " << msg.raw_len() << std::endl;
    std::cout << "\tmax content len : " << msg.max_content_len() << std::endl;
    std::cout << "\tcontent len     : " << msg.get_content_len() << std::endl;
    std::cout << "\tmessage type    : " << msg.get_type() << std::endl;
    std::cout << "\tshow message    : ";    msg.show_message(8);
    std::cout << "\tmessage content : " << msg << std::endl; 
 


    
    /* Message(usize sz, 
            const u8 content_buf[], 
            usize content_len,  
            u8 mtype)
  

    Message(usize sz,
            const std::string& str,
            u8 mtype):   Message(sz, (u8*) str.c_str(),  str.len(), mtype)
    {}

    

   
    msg.set_content_size(MESS_SIZE);
    msg.show_message(8);
    std::cout << std::endl;

    std::cout << "\n  -- demo load/unload str --\n";
    msg.set_type(MessageType::TEXT);
    auto contents = std::string("a test string");
    msg.set_content_str(contents);
    std::cout << "\n  contents: " << "\"" + contents + "\"";
    msg.show_message(8);
    auto sz = msg.get_content_size();
    std::cout << "\n  content size: " << sz;
    auto rslt = msg.get_content_str();
    if(!rslt.empty()) 
      std::cout << "\n  contents: " << rslt;
    std::cout << std::endl;

    std::cout << "\n  -- demo load/unload byte buffer --\n";
    msg.init();
    const u8 bytes[] = {1,2,3,4};
    msg.set_content_bytes(bytes, 4);
    std::cout << "\n  bytes: " << VecSlice(bytes,4);
    msg.show_message(8);
    sz = msg.get_content_size();
    std::cout << "\n  content size: " << sz;
    std::cout << "\n  bytes: " << msg.get_content_bytes();  // returns a stuctured binding and print with <<
    std::cout << std::endl;

    std::cout << "\n  -- demo setting MessageType --\n";
    msg.set_type(MessageType::TEXT);
    msg.show_message(8);
    auto mt = msg.get_type();
    std::cout << "\n  mt: " << (int) mt << " MessageType: " << msg.type_display();
    std::cout << std::endl; 

    std::cout << "\n  -- demo messages fitted to content --\n";
    msg = Message::create_msg_str_fit("a test string");
    msg.show_message(8);
    std::cout << "\n\n  content: " << msg.get_content_str();
    std::cout << std::endl;

    msg = Message::create_msg_bytes_fit(bytes, 4);
    msg.show_message(8);
    std::cout << "\n\n  content: " <<  msg.get_content_bytes();
    std::cout << std::endl;

    
    Message msg2; // default c_tor (has same semantics as "create_msg_bytes_fit(NULL,0)" below" )
    const u8* zerobuffer = nullptr; 
    // msg = Message::create_msg_bytes_fit(zerobuffer,0);  // intentionally 0 length
    msg2.show_message(8);
    std::cout << "\n\n  content: " << msg2.get_content_bytes();
    sz = msg2.get_content_size();
    std::cout << "\n\n  msg content size: " << sz;
    std::cout << std::endl;

    std::cout << "\n  -- demo header only message --\n";
    msg = Message::create_msg_header_only();
    sz = msg.get_content_size();
    msg.show_message(8);
    std::cout << "\n\n  msg content size: " << sz;
    
    std::cout << "\n\n  That's all Folks!\n\n";
    */
     
    return 0;
}

#endif
