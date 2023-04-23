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
    for (auto const& product : warehouse.products) {
      AProduct* aproduct = apurchase->add_things();
      aproduct->set_id(product.p_id);
      aproduct->set_count(product.p_num);
      aproduct->set_description(product.p_name);
    }
    trySendMsgToWorld(acommand, seq_num);
  }


  // for (auto const& warehouse : server.WH_list) {
  //     for (auto const& product : warehouse.products) {
  //         std::cout << "start to purchaseMore: " <<   std::endl;
  //         std::thread t_purchase(purchaseMore, warehouse.wh_id, product.p_id, product.p_name,
  //                       product.p_num);
  //                       t_purchase.detach();
  //     }
  // }



  // auto const& warehouse = server.WH_list[0];
  // auto const& product = warehouse.products[0];
  // std::cout << "start to purchaseMore: " <<   std::endl;
  // purchaseMore(warehouse.wh_id, product.p_id, product.p_name, product.p_num);
  // std::thread t_purchase(purchaseMore, warehouse.wh_id, product.p_id, product.p_name,
  //           product.p_num);
            // t_purchase.detach();
  // TODO: wait for all threads finishing
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

void processPacked(APacked& apacked){
  std::cout << "start to processPacked" << std::endl;
}

void processLoaded(ALoaded& aloaded){
  std::cout << "start to processLoaded" << std::endl;
}

void trySendMsgToWorld(ACommands& ac, int seq_num){
  // periodically thread, for at least once
  std::cout << "try to send MSG, for at least once: " <<  std::endl;
  Server& server = Server::getInstance();
  while (1){
      // std::cout << "start to periodically thread" <<  std::endl;
      server.A2W_send_queue.push(ac);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      if (server.finished_SeqNum_set.find(seq_num) != server.finished_SeqNum_set.end()){
        break;
      }
  }
  std::cout << "finished to periodically thread" <<  std::endl;
}
