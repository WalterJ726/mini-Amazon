#include "Server.hpp"

void listenFrontEndRequest(){
  // start to listen
  Server& server = Server::getInstance();
  while (1){
    int client_connection_fd = server.tryAccept();
    if (client_connection_fd == -1) {
      std::cout << "accpet failed" << std::endl;
      continue;
    }
    std::string recv_str_front_end = server.recvData(0);
    std::cout << recv_str_front_end << std::endl;
  }
}