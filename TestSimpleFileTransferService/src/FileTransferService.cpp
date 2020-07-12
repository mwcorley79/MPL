#include <mpl.h>

#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
#include "FileSystemLin.h"
#else
#include "FileSystemWin.h"
#endif

#include "Commands.h"
#include <sstream>

using namespace CSE384;
using namespace FileSystem;

// extend ClientHandler to get server processing
class FTSClientHandler : public ClientHandler
{
public:
  FTSClientHandler(const std::string &root_path, size_t msg_size);
  virtual ~FTSClientHandler();

  void DoSendFileList();
  void DoReceiveFile(const std::string &filename);
  void DoSendFile(const std::string &filename);
  void ProcessCommand(Message &msg);

  virtual void AppProc();
  virtual ClientHandler *Clone();

private:
  std::string root_path_;
  size_t msg_size_;
};

FTSClientHandler::FTSClientHandler(const std::string &root_path, size_t msg_size) : root_path_(root_path),
                                                                                    msg_size_(msg_size)
{
}

// need to implement Clone() to enable Receiver to instances on a per client basis
ClientHandler *FTSClientHandler::Clone()
{
  return new FTSClientHandler(root_path_, msg_size_);
}

FTSClientHandler::~FTSClientHandler()
{
  std::cout << "Server disconnected from client successfully" << std::endl;
}

void FTSClientHandler::DoSendFileList()
{
  std::vector<std::string> local_files = Directory::getFiles(root_path_, "*.*");
  for(auto f : local_files)
     PostMessage(Message(f, (int)Commands::REQUEST_FILE_LIST));
  PostMessage(Message(0, (int)Commands::ACK_FILE_LIST));
}

void FTSClientHandler::DoReceiveFile(const std::string &filename)
{
  Message msg;

  File file(root_path_ + filename);
  file.open(File::out, File::binary);

  if (file.isGood())
  {
    std::cout << "Uploading file from: " << filename
              << " from client: " << RemoteEP()
              << std::endl;

    Byte *begin, *end;

    //receive file blocks until client sends FILE_CLOSE message type
    while ((msg = GetMessage()).GetType() != (int)Commands::FILE_CLOSE)
    {
      Message &file_block = msg;
      begin = file_block.GetData();
      end = (file_block.GetData() + file_block.Length());
      file.putBlock(Block(begin, end));
    }

    file.close();

    std::cout << "Successfully Uploaded file: "
              << filename << " from client: " << RemoteEP()
              << std::endl;
  }
  else
  {
    std::cout << "Could not create file: " << filename << std::endl;
  }
}

void FTSClientHandler::DoSendFile(const std::string &filename)
{
  File file(root_path_ + filename);
  file.open(File::in, File::binary);

  if (!file.isGood())
  {
    std::string response_msg = std::string("File: ") + filename + std::string(" not found");
    std::cout << response_msg << std::endl;
    PostMessage(Message(response_msg, (int)Commands::FILE_NOT_FOUND));
    return;
  }

  std::cout << "Sending file: " << filename
            << " to " << RemoteEP() << std::endl;

  PostMessage(Message(filename, (int)Commands::FILE_RECV));

  while (file.isGood())
  {
    //chuck the file over the channel in fixed size messages
    Block b = file.getBlock(msg_size_);
    PostMessage(Message(&b[0], (int) b.size(), (int)Commands::FILE_BLOCK));
    std::cout << "Sent block of size: " << b.size()
              << " bytes\n"
              << std::endl;
  }

  file.close();

  //done, so send the FILE_CLOSE message
  PostMessage(Message(0, 0, (int)Commands::FILE_CLOSE));

  std::cout << "Sent File: " << file.name()
            << " to " << RemoteEP() << std::endl;
}

void FTSClientHandler::ProcessCommand(Message &msg)
{
  switch ((Commands)msg.GetType())
  {
  case Commands::REQUEST_FILE_LIST:
  {
    DoSendFileList();
  }
  break;

  case Commands::FILE_SEND:
  {
    std::string filename = msg.ToString();

    std::chrono::high_resolution_clock::time_point start =
        std::chrono::high_resolution_clock::now();
    DoSendFile(filename);

    std::chrono::high_resolution_clock::time_point stop =
        std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span =
        std::chrono::duration_cast<std::chrono::duration<double>>(stop - start);
    std::cout << "  Latency:= " << time_span.count() << std::endl;
  }
  break;

  case Commands::FILE_RECV:
  {
    std::string filename = msg.ToString();

    std::chrono::high_resolution_clock::time_point start =
        std::chrono::high_resolution_clock::now();
    DoReceiveFile(filename);

    std::chrono::high_resolution_clock::time_point stop =
        std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span =
        std::chrono::duration_cast<std::chrono::duration<double>>(stop - start);
    std::cout << "  Latency:= " << time_span.count() << std::endl;
  }
  }
}

void FTSClientHandler::AppProc()
{
  Message msg;
  // sout << locked << "Connection from: "<< RemoteEP() << MPL::endl << unlocked;
  while ((msg = GetMessage()).GetType() != MessageType::DISCONNECT)
    ProcessCommand(msg);
}

#ifdef TEST_SERVER
int main(int argc, char *argv[])
{

  if (argc != 5)
  {
    std::cout << "Usage:  fts_server  IP-address   Port   Msg-Size   [Upload-Download-Directory] " << std::endl;
    return 0;
  }
  EndPoint ep(argv[1], std::stoi(argv[2]));
  int msg_size = std::stoi(argv[3]);
  std::string root_path = std::string(argv[4]);

  //set SO_REUSEADDR socket option
  TCPSocketOptions sock_opts(SOL_SOCKET, SO_REUSEADDR);
  if (root_path[root_path.length() - 1] != '/')
    root_path += std::string("/");

  std::cout << "*************************************************************\n";
  std::cout << "********* File Transfer Service Version 1.0 **************** \n";
  std::cout << "*************************************************************\n";
  std::cout << "Rooted (Upload/Download) Path " << root_path << "\n";

  // create the and start the Receiver running with a ClientHandler
  FTSClientHandler fts_ch(root_path, (size_t) msg_size);

  Receiver recv(ep, &sock_opts);
  recv.RegisterClientHandler(&fts_ch);

  recv.Start();
  //recv.Stop();
}
#endif
