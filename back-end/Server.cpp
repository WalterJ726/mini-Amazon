#include "Server.hpp"
std::mutex mtx;
#define NUM_WH 5
#define zj78_host "vcm-30576.vm.duke.edu"

void Server::startRun() {
  std::cout << "start Run server" << std::endl;
  // Database db("exchange", "postgres", "passw0rd");
  // db.connect();
  // db.initialize();
  // db.disconnect();
  initWareHouse();
  initWorld();
  // recv msg from UPS (their hostname)
  
  // initialize the world, send AConnect

  // send msg to world simulator

  // recv response from world simulator

  // send msg to UPS

  // recv response from UPS

  // handle request from django customer
}

void Server::initWareHouse(){
  // initialized the product that shows in front end

}

void Server::initWorld(){
  GOOGLE_PROTOBUF_VERIFY_VERSION;  // use macro to check environment
  Client client = Client(23456, zj78_host);
  int world_fd = client.getSockfd();
  AConnect ac;
  if (world_id != -1) ac.set_worldid(world_id);

  for (int i = 0; i < NUM_WH; i ++ ){
    AInitWarehouse* ainit_wh = ac.add_initwh();
    ainit_wh->set_id(i);
    ainit_wh->set_x(i + 1);
    ainit_wh->set_y(i + 1);
  }
  ac.set_isamazon(true);
  // send AConnect to world
  std::unique_ptr<proto_out> world_out(new proto_out(world_fd));
  if (sendMesgTo<AConnect>(ac, world_out.get()) == false){
    std::cout << "failed to send msg to world" << std::endl;
    throw std::exception();
  }
  
  std::cout << "send msg to world successful" << std::endl;

  // get AConnected from world
  AConnected aconnected;
  std::unique_ptr<proto_in> world_in(new proto_in(world_fd));
  if (recvMesgFrom<AConnected>(aconnected, world_in.get()) == false){
    std::cout << "failed to recv msg from world" << std::endl;
    throw std::exception();
  }
  
  std::cout << "recv msg from world successful" << std::endl;

  if (aconnected.result() != "connected!"){
    std::cout << "fail to connected" << std::endl;
    throw std::exception();
  }

  int connected_world_id = aconnected.worldid();
  std::cout << "connected to world: " << connected_world_id <<  std::endl;
}


Server::Server() : port_num(8888), world_id(-1) {
  hasError = 0;
  if (setUpStruct() == -1) {
    hasError = 1;
  }
  if (initSocketFd() == -1) {
    hasError = 1;
  }
  if (tryBind() == -1) {
    hasError = 1;
  }
  if (startListen() == -1) {
    hasError = 1;
  }
}

int Server::setUpStruct() {
  int status;
  memset(&host_info, 0, sizeof(host_info));

  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  host_info.ai_flags = AI_PASSIVE;
  std::string port_str = std::to_string(port_num);
  status = getaddrinfo(nullptr, port_str.c_str(), &host_info, &host_info_list);
  if (status != 0) {
    // "Error: cannot get address info for host"
    return -1;
  }
  return status;
}

int Server::initSocketFd() {
  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    // "Error: cannot create socket" << endl;
    return -1;
  }
  return socket_fd;
}

int Server::tryBind() {
  int status;
  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    // "Error: cannot bind socket" << endl;
    return -1;
  }
  return status;
}

int Server::startListen() {
  int status;
  status = listen(socket_fd, 100);
  if (status == -1) {
    // "Error: cannot listen on socket"
    return -1;
  }
  freeaddrinfo(host_info_list);
  return status;
}

int Server::tryAccept() {
  struct sockaddr_storage socket_addr;
  socklen_t socket_addr_len = sizeof(socket_addr);
  client_connection_fd =
      accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  if (client_connection_fd == -1) {
    // "Error: cannot accept connection on socket"
    return -1;
  }
  return client_connection_fd;
}

int Server::getErrorSign() {
  return hasError;
}

std::string Server::recvData(int flag) {
  char recvbuff[MAX_TCP_PACKET_SIZE];
  int numbytes;
  if ((numbytes = recv(client_connection_fd, recvbuff, MAX_TCP_PACKET_SIZE, flag)) ==
      -1) {
    hasError = 1;
    return nullptr;
  }
  // recvbuff[numbytes] = '\0';
  return std::string(recvbuff, numbytes);
}

bool Server::sendAllData(int sockfd, const char * msg, size_t size) {
  size_t numBytes = 0;
  size_t bytesleft = size;
  int recvBytes = 0;
  while ((numBytes < bytesleft)) {
    if ((recvBytes = send(sockfd, msg + numBytes, size, MSG_NOSIGNAL)) == -1) {
      perror("client send");
      break;
    }
    numBytes += recvBytes;
    bytesleft -= recvBytes;
  }

  return recvBytes == -1 ? false : true;
}
