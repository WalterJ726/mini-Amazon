#include "handleWorld.hpp"

HandleWorld::HandleWorld(const AResponses & aresponses){
  for (int i = 0; i < aresponses.arrived_size(); i++) {
    apurchasemores.push_back(std::move(aresponses.arrived(i)));
    seqNums.push_back(aresponses.arrived(i).seqnum());
  }

  for (int i = 0; i < aresponses.ready_size(); i++) {
    apackeds.push_back(std::move(aresponses.ready(i)));
    seqNums.push_back(aresponses.ready(i).seqnum());
  }

  for (int i = 0; i < aresponses.loaded_size(); i++) {
    aloadeds.push_back(std::move(aresponses.loaded(i)));
    seqNums.push_back(aresponses.loaded(i).seqnum());
  }
  
  Server& server = Server::getInstance();
  // record acks from world
  for (int i = 0; i < aresponses.acks_size(); i ++ ){
    std::cout << "aresponses.acks(i) is " << aresponses.acks(i) << std::endl;
    server.global_finished_SeqNum_set.insert(aresponses.acks(i));
  }
}

void initProductsAmount(){
  Server& server = Server::getInstance();
  // buy some initial products
  for (auto const& warehouse : server.WH_list) {
      ACommands acommand;
      APurchaseMore* apurchase = acommand.add_buy();
      Server& server = Server::getInstance();
      int seq_num = server.getSeqNum();
      apurchase->set_seqnum(seq_num);
      apurchase->set_whnum(warehouse.wh_id);
      AProduct* aproduct = apurchase->add_things();
      aproduct->set_id(warehouse.products.p_id);
      aproduct->set_count(warehouse.products.p_num);
      aproduct->set_description(warehouse.products.p_name);
      trySendMsgToWorld(acommand, seq_num);
  }
}

bool checkWorldHasHandled(int seqnum){
  Server& server = Server::getInstance();
  std::cout << "seqnum() is " << seqnum << std::endl;
  if (server.finished_SeqNum_set.find(seqnum) != server.finished_SeqNum_set.end()){
    return true;
  }
  server.finished_SeqNum_set.insert(seqnum);
  return false;
}

void HandleWorld::handleWorldResponse(){
      Server& server = Server::getInstance();
      // ACK responses to world.
      ACommands all_acks;
      for (size_t i = 0; i < seqNums.size(); i++) {
        all_acks.add_acks(i);
        all_acks.set_acks(i, seqNums[i]);
      }
      server.A2W_send_queue.push(all_acks);

      // start to parse APurchaseMore
      for (size_t i = 0; i < apurchasemores.size(); i ++ ){
        APurchaseMore arrived = apurchasemores[i];
        int seqnum = arrived.seqnum();
        if (!checkWorldHasHandled(seqnum)){
          std::cout << "start to parse APurchaseMore" << std::endl;
          processPurchaseMore(arrived);
        }
      }

      // start to parse APacked
      for (size_t i = 0; i < apackeds.size(); i ++ ){
        APacked ready = apackeds[i];
        int seqnum = ready.seqnum();
        if (!checkWorldHasHandled(seqnum)){
          std::cout << "start to parse APacked" << std::endl;
          processPacked(ready);
        }
      }

      // start to parse ALoaded
      for (size_t i = 0; i < aloadeds.size(); i ++ ){
        ALoaded loaded = aloadeds[i];
        int seqnum = loaded.seqnum();
        if (!checkWorldHasHandled(seqnum)){
          std::cout << "start to parse ALoaded" << std::endl;
          processLoaded(loaded);
        }
      }
}

void purchaseMore(const int wh_id, const int p_id, const std::string p_name, const int p_num){
  std::cout << "start to purchaseMore: at wh_id " << wh_id << " of p_id " << p_id <<  std::endl;
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

void processPurchaseMore(APurchaseMore apurchasemore){
    // parse whnum, products
    Database& db = Database::getInstance();
    std::cout << "start to processPurchaseMore" << std::endl;
    int wh_id = apurchasemore.whnum();
    for (int i = 0; i < apurchasemore.things_size(); i ++ ){
      long p_id = apurchasemore.things(i).id();
      std::string p_name = apurchasemore.things(i).description();
      int p_num = apurchasemore.things(i).count();
      // add this product to Inventory
      std::cout << "add " << p_id << " this product to Inventory " << wh_id << std::endl;
      db.insert_and_update_inventory(wh_id, p_id, p_num);
    }
    std::cout << "success add products into warehouse" << std::endl;
}

void processPacked(APacked apacked){
  std::cout << "start to processPacked" << std::endl;
  int ship_id = apacked.shipid();
  Database& db = Database::getInstance();
  std::string status = "packed";
  db.update_package_status(ship_id, status);
}

void processLoaded(ALoaded aloaded){
  std::cout << "start to processLoaded" << std::endl;
  int ship_id = aloaded.shipid();
  Database& db = Database::getInstance();
  std::string status = "loadeded";
  db.update_package_status(ship_id, status);
  // try send AUreqDelivery to UPS
  sendUPSReqDelivery(ship_id);
}

void trySendMsgToWorld(ACommands& ac, int seq_num){
  // periodically thread, for at least once
  std::cout << "try to send MSG, for at least once: " <<  std::endl;
  Server& server = Server::getInstance();
  while (1){
      // std::cout << "start to periodically thread" <<  std::endl;
      server.A2W_send_queue.push(ac);
      std::cout << ac.DebugString() << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      if (server.global_finished_SeqNum_set.find(seq_num) != server.global_finished_SeqNum_set.end()){
        break;
      }
  }
  std::cout << "finished to periodically thread" <<  std::endl;
}
