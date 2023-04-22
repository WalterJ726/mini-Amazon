#include "Server.hpp"
std::mutex mtx;
#define zj78_host "vcm-30576.vm.duke.edu"

void Server::startRun() {
  std::cout << "start Run server" << std::endl;
  // Database db("exchange", "postgres", "passw0rd");
  // db.connect();
  // db.initialize();
  // db.disconnect();
  initWareHouse();
  // recv msg from UPS (their hostname)
  
  // initialize the world, send AConnect
  initWorld();
  // send msg to world simulator
  std::thread t_A2W_request(sendMsgToWorld);
  t_A2W_request.detach();
  // recv response from world simulator
  std::thread t_W2A_response(recvMsgFromWorld);
  t_W2A_response.detach();
  // send msg to UPS

  // recv response from UPS

  // handle request from django customer
}

void Server::initWareHouse(){
  // initialized the product that shows in front end
  for (int i = 0; i < NUM_WH; i ++ ){
    WareHouse wh;
    wh.wh_id = i;
    wh.loc_x = i + 1;
    wh.loc_y = i + 1;
    WH_list.push_back(wh);
  }
}

void Server::initWorld(){
  GOOGLE_PROTOBUF_VERIFY_VERSION;  // use macro to check environment
  Client client = Client(23456, zj78_host);
  world_fd = client.getSockfd();
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

  // buy some initial products
  for (auto const& warehouse : WH_list) {
      for (auto const& product : warehouse.products) {
          std::thread t_purchase(purchaseMore, warehouse.wh_id, product.p_id, product.p_name,
                        product.p_num);
      }
  }
  // TODO: wait for all threads finishing
}

void Server::purchaseMore(const int wh_id, const int p_id, const std::string p_name, const int p_num){
  ACommands acommand;
  APurchaseMore* apurchase = acommand.add_buy();
  AProduct* aproduct = apurchase->add_things();
  aproduct->set_id(p_id);
  aproduct->set_count(p_num);
  aproduct->set_description(p_name);
  apurchase->set_whnum(wh_id);
  Server& server = Server::getInstance();
  int seq_num = server.getSeqNum();
  apurchase->set_seqnum(seq_num);
  trySendMsgToWorld(acommand, seq_num);
}


void Server::sendMsgToWorld(){
  Server& server = Server::getInstance();
  std::unique_ptr<proto_out> world_out(new proto_out(server.world_fd));
  while (1){
    if (!server.A2W_send_queue.empty()){
      ACommands acommand = server.A2W_send_queue.front();
      // send AConnect to world
      if (sendMesgTo<ACommands>(acommand, world_out.get()) == false){
        std::cout << "failed to send msg to world in sendMsgToWorld()" << std::endl;
        continue;
      }
      std::cout << "send msg to world successful in sendMsgToWorld()" << std::endl;
    }
  }
}

void Server::recvMsgFromWorld(){
  // get AResponses from world
    AResponses aresponses;
    Server& server = Server::getInstance();
    std::unique_ptr<proto_in> world_in(new proto_in(server.world_fd));
    while (1){
    if (recvMesgFrom<AResponses>(aresponses, world_in.get()) == false){
      std::cout << "failed to recv msg from world in recvMsgFromWorld()" << std::endl;
      continue;
    }

      std::cout << "recv msg from world successful in recvMsgFromWorld()" << std::endl;
      for (int i = 0; i < aresponses.acks_size(); i ++ ){
        if (server.finished_SeqNum_set.find(aresponses.acks(i)) != server.finished_SeqNum_set.end()){
          continue;
        }
        server.finished_SeqNum_set.insert(aresponses.acks(i));
      }
    

      // start to parse APurchaseMore
      for (int i = 0; i < aresponses.arrived_size(); i ++ ){
        APurchaseMore arrived = aresponses.arrived(i);
        int seqnum = arrived.seqnum();
        if (server.finished_SeqNum_set.find(seqnum) != server.finished_SeqNum_set.end()){
          continue;
        }
        processPurchaseMore(arrived);
        server.finished_SeqNum_set.insert(seqnum); 
      }

      // start to parse APacked
      for (int i = 0; i < aresponses.ready_size(); i ++ ){
        APacked ready = aresponses.ready(i);
        int seqnum = ready.seqnum();
        if (server.finished_SeqNum_set.find(seqnum) != server.finished_SeqNum_set.end()){
          continue;
        }
        processPacked(ready);
        server.finished_SeqNum_set.insert(seqnum); 
      }

      // start to parse ALoaded
      for (int i = 0; i < aresponses.loaded_size(); i ++ ){
        ALoaded loaded = aresponses.loaded(i);
        int seqnum = loaded.seqnum();
        if (server.finished_SeqNum_set.find(seqnum) != server.finished_SeqNum_set.end()){
          continue;
        }
        processLoaded(loaded);
        server.finished_SeqNum_set.insert(seqnum); 
      }
    }
}

void Server::trySendMsgToWorld(ACommands& ac, int seq_num){
  // periodically thread, for at least once
  Server& server = Server::getInstance();
  while (1){
      server.A2W_send_queue.push(ac);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      if (server.finished_SeqNum_set.find(seq_num) != server.finished_SeqNum_set.end()){
        break;
      }
  }
}

void Server::processPurchaseMore(APurchaseMore& apurchasemore){
    // parse whnum, products
    std::cout << "start to processPurchaseMore" << std::endl;
    int wh_id = apurchasemore.whnum();
    for (int i = 0; i < apurchasemore.things_size(); i ++ ){
      long p_id = apurchasemore.things(i).id();
      std::string p_name = apurchasemore.things(i).description();
      int p_num = apurchasemore.things(i).count();
      // add this product to Inventory
      std::cout << "add " << p_id << " this product to Inventory " << wh_id << std::endl;
    }
    // add products to whnum

    // send ack back to the world
    std::cout << "success add products into warehouse" << std::endl;
}

void Server::processPacked(APacked& apacked){
  std::cout << "start to processPacked" << std::endl;
}

void Server::processLoaded(ALoaded& aloaded){
  std::cout << "start to processLoaded" << std::endl;
}

long Server::getSeqNum(){
  std::lock_guard<std::mutex> lck (mtx);
  return SeqNum++;
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
