#include <future>
#include <vector>
#include <chrono>
#include <ratio>
#include <ctime>
#include <iostream>
#include <string>

#include <mpl.h>

#if !defined(WIN32) && !defined(_WIN32) && !defined(__WIN32__) && !defined(__NT__) && !defined(_WIN64)
#include "FileSystemLin.h"
#else
#include "FileSystemWin.h"
#endif

#include "Commands.h"

using namespace CSE384;
using namespace FileSystem;

// inherits implementation of FixedSizeMsgSender to gain Fixed size messaging.
// PostMessage(), SendPessage, GetMessage() functions etc.
class FTSClient : private Sender
{
public:
   FTSClient(const std::string &ip,
             int port,
             int msg_size,
             const std::string &rootpath,
             TCPSocketOptions *sock_opts);

   void DisplayMenu();
   void DoClientConnection();
   std::vector<std::string> DoGetRemoteFileList();
   void UpdateDisplay(const std::string &command, std::vector<std::string> &files);
   void DoSendFile(const std::string &filename);
   void DoReceiveFile(const std::string &rfile);
   bool ProcessCommand(const std::string &command);

private:
   std::string root_path_;
   EndPoint server_ep_;
   size_t msg_size_;
};

FTSClient::FTSClient(const std::string &ip,
                     int port,
                     int msg_size,
                     const std::string &rootpath,
                     TCPSocketOptions *sock_opts) : Sender(sock_opts),
                                                    root_path_(rootpath),
                                                    server_ep_(ip, port),
                                                    msg_size_(msg_size)

{
}

void FTSClient::DisplayMenu()
{
   std::cout << std::endl;
   std::cout << "*********************************************************" << std::endl;
   std::cout << "FTS Client Version 1.0 " << std::endl;
   std::cout << "Rooted (Upload/Download) Path " << root_path_ << std::endl;
   std::cout << "*********************************************************" << std::endl;
   std::cout << "L            - list files client (local)                 " << std::endl;
   std::cout << "R            - list files server (remote)                " << std::endl;
   std::cout << "G [filename] - get a file from the remote server         " << std::endl;
   std::cout << "P [filename] - put a file to the remote server           " << std::endl;
   std::cout << "Q            - close the connection and exit             " << std::endl;
   std::cout << "*********************************************************" << std::endl;
   std::cout << "Enter command :=> ";
   std::cout << std::endl;
}

void FTSClient::DoClientConnection()
{
   bool done = false;
   std::string input;

   // attempt to connect upto 10 ten times with 1 sec. interval between
   // attempts
   ConnectPersist(server_ep_, 10, 1, 2);

   if (IsConnected())
   {

      DisplayMenu();
      while (!done)
      {
         std::getline(std::cin, input);
         done = ProcessCommand(input);
         DisplayMenu();
      }
   }

   //close the sender
   Close();
}

std::vector<std::string> FTSClient::DoGetRemoteFileList()
{
   //file list request message
   Message file_list_request(Message(0, 0, (int)Commands::REQUEST_FILE_LIST));

   //dispatch request message to server
   PostMessage(file_list_request);

   //receive response messages and return list of remote files
   Message response_msg;
   std::vector<std::string> remote_files;

   while ((response_msg = GetMessage()).GetType() != (int)(Commands::ACK_FILE_LIST))
   {
      remote_files.push_back(response_msg.ToString());
   }

   return remote_files;
}

void FTSClient::UpdateDisplay(const std::string &command, std::vector<std::string> &files)
{
   if (command == "R")
      std::cout << "File(s) On Remote Server at: " << server_ep_ << std::endl;
   else
      std::cout << "Files(s) in Local Rooted Path: "
                << Path::getFullFileSpec(root_path_) << std::endl;

   for (auto f : files)
      std::cout << "\t" << f << std::endl;
   std::cout << std::endl;
}

void FTSClient::DoSendFile(const std::string &filename)
{
   File file(root_path_ + filename);
   file.open(File::in, File::binary);

   if (!file.isGood())
   {
      std::cout << "File: " << filename << " not found" << std::endl;
      return;
   }

   std::cout << "Uploading File: " << filename << " to " << server_ep_ << std::endl;

   // dispatch the file create message
   PostMessage(Message(filename, (int)Commands::FILE_RECV));

   //dispatch file in fixed size blocks: FixedSizeMsgSender::GetMsgDataSize()
   while (file.isGood())
   {
      Block b = file.getBlock(msg_size_);
      if (b.size())
         PostMessage(Message(&b[0], b.size(), (int)Commands::FILE_BLOCK));
   }

   file.close();

   //dispatch the file close message
   PostMessage(Message(0, 0, (int)Commands::FILE_CLOSE));

   std::cout << "Uploaded file: " << file.name() << " to "
             << server_ep_ << std::endl;
}

void FTSClient::DoReceiveFile(const std::string &rfile)
{
   // dispatch the request to the server to send the remote file
   Message file_request(rfile, (int)Commands::FILE_SEND);
   PostMessage(file_request);

   //now receive the response from the server: (the file requested, or not found)
   Message msg;
   if ((msg = GetMessage()).GetType() == (int)Commands::FILE_RECV)
   {
      std::string filename = msg.ToString();
      File file(root_path_ + filename);
      file.open(File::out, File::binary);

      if (file.isGood())
      {
         std::cout << "Downloading File: " << filename << std::endl;

         while ((msg = GetMessage()).GetType() != (int)Commands::FILE_CLOSE)
            file.putBlock(Block(msg.GetData(), (msg.GetData() + msg.Length())));
         file.close();

         std::cout << "Downloaded file: " << root_path_ + filename << std::endl;
      }
   }
   else if (msg.GetType() == (int)Commands::FILE_NOT_FOUND)
   {
      std::cout << "Response from Server: " << msg.ToString() << std::endl;
   }
}

bool FTSClient::ProcessCommand(const std::string &command)
{
   std::istringstream iss(command);
   std::string cmd, arg = "none";
   iss >> cmd >> arg;
   for (int i = 0; i < cmd.length(); ++i)
      cmd[i] = std::toupper(cmd[i]);

   if (cmd == "Q")
      return true;

   if (cmd == "R")
   {
      std::vector<std::string> remote_files = std::move(DoGetRemoteFileList());
      UpdateDisplay("R", remote_files);
   }
   else if (cmd == "L")
   {
      std::vector<std::string> local_files = FileSystem::Directory::getFiles(root_path_, "*.*");
      UpdateDisplay("L", local_files);
   }
   else if (cmd == "G" && arg != "none")
   {
      std::chrono::high_resolution_clock::time_point start =
          std::chrono::high_resolution_clock::now();
      DoReceiveFile(arg); // *** receive the file

      std::chrono::high_resolution_clock::time_point stop =
          std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span =
          std::chrono::duration_cast<std::chrono::duration<double>>(stop - start);
      std::cout << "  Latency:= " << time_span.count() << std::endl;
   }
   else if (cmd == "P" && arg != "none")
   {
      std::chrono::high_resolution_clock::time_point start =
          std::chrono::high_resolution_clock::now();
      DoSendFile(arg); // *** send the file

      std::chrono::high_resolution_clock::time_point stop =
          std::chrono::high_resolution_clock::now();
      std::chrono::duration<double> time_span =
          std::chrono::duration_cast<std::chrono::duration<double>>(stop - start);
      std::cout << "  Latency:= " << time_span.count() << std::endl;
   }
   else
      std::cout << "Bad Syntax!\n"
                << std::endl;

   return false;
}

#ifdef TEST_CLIENT

int main(int argc, char *argv[])
{
   if (argc != 5)
   {
      std::cerr << "Usage: fts_client  ip-address  port  msg-size  rooted-path" << std::endl;
      return 0;
   }

   std::string root_path = std::string(argv[4]);
   if (root_path[root_path.length() - 1] != '/')
      root_path += std::string("/");

   TCPSocketOptions sock_opts(SOL_SOCKET, SO_KEEPALIVE);
   FTSClient ftsc(argv[1], std::stoi(argv[2]), std::stoi(argv[3]), root_path, &sock_opts);
   ftsc.DoClientConnection();
   return 0;
}
#endif