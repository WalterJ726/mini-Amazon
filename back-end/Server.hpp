#ifndef _SERVER_HPP
#define _SERVER_HPP
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mutex>
#include <memory>
#include <queue>
#include <thread>
#include <chrono>
#include "handleProto.hpp"
#include "handleWorld.hpp"
#include "Database.hpp"
#include "threadsafe_queue.h"
#include "WareHouse.hpp"
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
  long SeqNum;
  int world_fd;

  
private:
	Server();
  ~Server() {};
	Server(const Server&);
	Server& operator=(const Server&);
 public:
  // ware house
  std::vector<WareHouse> WH_list;
  ThreadSafe_queue<ACommands> A2W_send_queue;
  std::unordered_set<int> finished_SeqNum_set;

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
  
  // get seqNum
  long getSeqNum();
  
  // handle request
  void sendMsgToWorld();

  // handle response 
  void recvMsgFromWorld();
  // periodically thread
  static void trySendMsgToWorld(ACommands ac, int seq_num);
  // int connectToServer();


  // void listenFrontEndRequest();
};

void listenFrontEndRequest();
#endif // _SERVER_HPP