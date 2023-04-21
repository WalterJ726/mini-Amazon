#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include "protobuf/world_amazon.pb.h"
#include "handleProto.hpp"
#include "client.hpp"

#define MAX_TCP_PACKET_SIZE 65535

class Server {
 private:
 // socket variable
  int hasError;
  int socket_fd;
  int client_connection_fd;
  const int port_num;
  struct addrinfo host_info;
  struct addrinfo * host_info_list;

  // event variable
  int world_id; 

  
private:
	Server();
	~Server() {};
	Server(const Server&);
	Server& operator=(const Server&);

 public:
  void startRun();

  // create a socket to listen
  int setUpStruct();
  int initSocketFd();
  int tryBind();
  int startListen();

  // try accept
  int tryAccept();
  // send and recv data
  std::string recvData(int flag);
  static bool sendAllData(int sockfd, const char * msg, size_t size);
  int getErrorSign();


  // Singleton
	static Server& getInstance() {
    // adapted from Effective C++ using Singleton
		static Server instance; // magic static
		return instance;
	}
  // initialized world
  void initWareHouse();
  void initWorld();
  
  // handle request


  // int connectToServer();
};
