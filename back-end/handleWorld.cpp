#include "handleWorld.hpp"

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

void handleWorldResponse(AResponses& aresponses){
      Server& server = Server::getInstance();
      std::cout << aresponses.DebugString() << std::endl;
      for (int i = 0; i < aresponses.acks_size(); i ++ ){
        std::cout << "aresponses.acks(i) is " << aresponses.acks(i) << std::endl;
        server.finished_SeqNum_set.insert(aresponses.acks(i));
      }
    
      // start to parse APurchaseMore
      for (int i = 0; i < aresponses.arrived_size(); i ++ ){
        ACommands APurchaseMore_ack;
        APurchaseMore arrived = aresponses.arrived(i);
        int seqnum = arrived.seqnum();
        std::cout << "arrived.seqnum() is " << seqnum << std::endl;
        if (server.finished_SeqNum_set.find(seqnum) != server.finished_SeqNum_set.end()){
          continue;
        }
        std::cout << "start to parse APurchaseMore" << std::endl;
        APurchaseMore_ack.add_acks(seqnum);
        server.A2W_send_queue.push(APurchaseMore_ack);
        processPurchaseMore(arrived);
      }

      // start to parse APacked
      for (int i = 0; i < aresponses.ready_size(); i ++ ){
        ACommands APacked_ack;
        APacked ready = aresponses.ready(i);
        int seqnum = ready.seqnum();
        if (server.finished_SeqNum_set.find(seqnum) != server.finished_SeqNum_set.end()){
          continue;
        }
        std::cout << "start to parse APacked" << std::endl;
        APacked_ack.add_acks(seqnum);
        server.A2W_send_queue.push(APacked_ack);
        processPacked(ready);
      }

      // start to parse ALoaded
      for (int i = 0; i < aresponses.loaded_size(); i ++ ){
        ACommands ALoaded_ack;
        ALoaded loaded = aresponses.loaded(i);
        int seqnum = loaded.seqnum();
        if (server.finished_SeqNum_set.find(seqnum) != server.finished_SeqNum_set.end()){
          continue;
        }
        std::cout << "start to parse ALoaded" << std::endl;
        ALoaded_ack.add_acks(seqnum);
        server.A2W_send_queue.push(ALoaded_ack);
        processLoaded(loaded);
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

void processPurchaseMore(APurchaseMore& apurchasemore){
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

void processPacked(APacked& apacked){
  std::cout << "start to processPacked" << std::endl;
  int ship_id = apacked.shipid();
  Database& db = Database::getInstance();
  std::string status = "packed";
  db.update_package_status(ship_id, status);
}

void processLoaded(ALoaded& aloaded){
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
      if (server.finished_SeqNum_set.find(seq_num) != server.finished_SeqNum_set.end()){
        break;
      }
  }
  std::cout << "finished to periodically thread" <<  std::endl;
}
