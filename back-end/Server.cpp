#include "Server.hpp"
std::mutex mtx;
#define zj78_host "vcm-30576.vm.duke.edu"
Client client(23456, zj78_host);

void Server::startRun() {
  std::cout << "start Run server" << std::endl;
  Database& db = Database::getInstance();
  // db.connect();
  // db.initialize();
  // db.disconnect();
  try
  {
    // client = Client(23456, zj78_host);
    world_fd = client.getSockfd();
    initWareHouse();
    // recv msg from UPS (their hostname)
    // initialize the world, send AConnect
    initWorld();
    // recv response from world simulator
    std::thread t_W2A_response(&Server::recvMsgFromWorld, this);
    // send msg to world simulator
    std::thread t_A2W_request(&Server::sendMsgToWorld, this);
    // t_A2W_request.detach();
    // t_W2A_response.detach();

    // // recv response from UPS
    // std::thread t_U2A_response(&Server::recvMsgFromUPS, this);

    // // send msg to UPS
    // std::thread t_A2U_request(&Server::sendMsgToUPS, this);


    // initlize products
    // initProductsAmount();

    // handle request from django customer
    listenFrontEndRequest();
    t_W2A_response.join();
    t_A2W_request.join();
  }
  catch(const std::exception& e)
  {
    close(world_fd);
    std::cerr << e.what() << '\n';
    return;
  }

}

void Server::initWareHouse(){
  // initialized the product that shows in front end
  Database& db = Database::getInstance();
  db.connect();
  // db.initialize();
  for (int i = 0; i < NUM_PRODUCT; i ++ ){
    db.insert_and_update_product(i, std::to_string(i), std::to_string(i));
  }
  for (int i = 0; i < NUM_WH; i ++ ){
    WareHouse wh;
    wh.wh_id = i;
    wh.loc_x = i + 1;
    wh.loc_y = i + 1;
    std::cout << "start to init ware house, wh_id: " << wh.wh_id << std::endl;
    WH_list.push_back(wh);
    db.insert_and_update_warehouse(wh.wh_id, wh.loc_x, wh.loc_y);
    
    if (i == 0){
        for (int j = 0; j < NUM_PRODUCT; j ++ ){
          db.insert_and_update_product(j, wh.products[j].p_name, wh.products[j].p_name);
          db.initialize_inventory(wh.wh_id, j, PRODUCT_INIT_NUM);
        }
    }
  }
}

void Server::initWorld(){
  GOOGLE_PROTOBUF_VERIFY_VERSION;  // use macro to check environment
  AConnect ac;
  if (world_id != -1) ac.set_worldid(world_id);

  for (int i = 0; i < NUM_WH; i ++ ){
    AInitWarehouse* ainit_wh = ac.add_initwh();
    ainit_wh->set_id(WH_list[i].wh_id);
    ainit_wh->set_x(WH_list[i].loc_x);
    ainit_wh->set_y(WH_list[i].loc_y);
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

void Server::sendMsgToWorld(){
  Server& server = Server::getInstance();
  std::unique_ptr<proto_out> world_out(new proto_out(world_fd));
  while (1){
    std::cout << "start to sendMsgToWorld()" << std::endl;
      ACommands acommand;
      A2W_send_queue.wait_and_pop(acommand);
      // send AConnect to world
      if (sendMesgTo<ACommands>(acommand, world_out.get()) == false){
        std::cout << "failed to send msg to world in sendMsgToWorld()" << std::endl;
        throw std::exception();
      }
      std::cout << "send msg to world successful in sendMsgToWorld()" << std::endl;
  }
}

void Server::recvMsgFromWorld(){
  // get AResponses from world
    Server& server = Server::getInstance();
    std::unique_ptr<proto_in> world_in(new proto_in(server.world_fd));
    while (1){
      AResponses aresponses;
      if (recvMesgFrom<AResponses>(aresponses, world_in.get()) == false){
        // std::cout << "failed to recv msg from world in recvMsgFromWorld()" << std::endl;
        continue;
      }
      std::cout << "recv msg from world successful in recvMsgFromWorld()" << std::endl;
      handleWorldResponse(aresponses);
    }
}


// void Server::sendMsgToUPS(){
//   Server& server = Server::getInstance();
//   std::unique_ptr<proto_out> world_out(new proto_out(world_fd));
//   while (1){
//     std::cout << "start to sendMsgToWorld()" << std::endl;
//       ACommands acommand;
//       A2W_send_queue.wait_and_pop(acommand);
//       // send AConnect to world
//       if (sendMesgTo<ACommands>(acommand, world_out.get()) == false){
//         std::cout << "failed to send msg to world in sendMsgToWorld()" << std::endl;
//         throw std::exception();
//       }
//       std::cout << "send msg to world successful in sendMsgToWorld()" << std::endl;
//   }
// }

// void Server::recvMsgFromUPS(){
//   // get AResponses from world
//     Server& server = Server::getInstance();
//     std::unique_ptr<proto_in> world_in(new proto_in(server.world_fd));
//     while (1){
//       AResponses aresponses;
//       if (recvMesgFrom<AResponses>(aresponses, world_in.get()) == false){
//         // std::cout << "failed to recv msg from world in recvMsgFromWorld()" << std::endl;
//         continue;
//       }
//       std::cout << "recv msg from world successful in recvMsgFromWorld()" << std::endl;
//       handleWorldResponse(aresponses);
//     }
// }


long Server::getSeqNum(){
  std::lock_guard<std::mutex> lck (mtx);
  return SeqNum++;
}

long Server::getOrderNum(){
  std::lock_guard<std::mutex> lck (mtx);
  return OrderNum++;
}

Server::Server() : port_num(6969), world_id(-1) {
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
