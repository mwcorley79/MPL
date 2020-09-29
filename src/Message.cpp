#ifdef TEST_MESSAGE
#include "Message.h"
#include <iostream>
#include <assert.h>
#include <string>
using namespace CSE384;

int main()
{
    const usize MESS_SIZE = 32;
    std::cout << "\n  -- demp Message type --\n ";
   
    std::cout << "\n  -- demo writing directly to msg buffer --\n";
    Message msg(MESS_SIZE);
    std::cout << "\n msg len: " << msg.get_ref().size(); 
    for(auto i = TYPE_SIZE + CONTENT_SIZE; i < MESS_SIZE; ++i) {
       msg.get_mut_ref()[i] = u8 (i);
    }
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

    const u8* zerobuffer = nullptr; 
    msg = Message::create_msg_bytes_fit(zerobuffer,0);  // intentionally 0 length
    msg.show_message(8);
    std::cout << "\n\n  content: " << msg.get_content_bytes();
    sz = msg.get_content_size();
    std::cout << "\n\n  msg content size: " << sz;
    std::cout << std::endl;

    std::cout << "\n  -- demo header only message --\n";
    msg = Message::create_msg_header_only();
    sz = msg.get_content_size();
    msg.show_message(8);
    std::cout << "\n\n  msg content size: " << sz;
    
    std::cout << "\n\n  That's all Folks!\n\n";
     
    return 0;
}

#endif
