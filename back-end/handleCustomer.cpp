#include "handleCustomer.h"

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
    ServerRequest customer_request(recv_str_front_end);
    std::string action = customer_request.getAction();
    if (action == "bind"){
      // start to process bind
    } else {
      // start to process order
      std::map<std::string, std::vector<std::string>> headerMap = customer_request.getHeaderMap();
      // server.sendAllData(client_connection_fd, action.c_str(), action.size());
      for (auto it = headerMap.begin(); it != headerMap.end(); ++it) {
          std::cout << it->first << std::endl;
          for (int i = 0; i < it->second.size(); i ++ ){
              std::cout << it->second[i];
          }
          std::cout << std::endl;
      }
    }
    close(client_connection_fd);
  }
}