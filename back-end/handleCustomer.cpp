#include "handleCustomer.h"

size_t get_user_id_order(const std::map<std::string, std::vector<std::string>> & headerMap){
  try{
    return std::stoul(headerMap.at("user_id").at(0));
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    std::cout << "user_id is invalid when using get_user_id_order to parse it" << std::endl;
    throw;
  }
}

string get_user_name_order(const std::map<std::string, std::vector<std::string>> & headerMap){
  try{
    return headerMap.at("user_name").at(0);
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    std::cout << "user_name is invalid when using get_user_name_order to parse it" << std::endl;
    throw;
  }
}

std::vector<std::pair<std::pair<size_t, std::string>, size_t>> get_products_order(const std::map<std::string, std::vector<std::string>> & headerMap){
  try{
    size_t product_types = headerMap.at("product_id").size();
    std::vector<std::pair<std::pair<size_t, std::string>, size_t>> products;
    
    for (size_t i = 0; i < product_types; i++){
      std::pair<size_t, std::string> id_name(std::stoul(headerMap.at("product_id").at(i)), headerMap.at("product_id").at(i));
      size_t quantity = std::stoul(headerMap.at("quantity").at(i));
      products.push_back(std::pair<std::pair<size_t, std::string>, size_t>(id_name, quantity));
    }

    return products;
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    std::cout << "product_id, product_name or quantity is invalid when using get_products_order to parse it" << std::endl;
    throw;
  }
}

void send_to_world_buy_mroe(size_t warehouse_id, size_t product_id, std::string description, size_t quantity){
  // ACommands buy_more;
  // APurchaseMore* apurchase = buy_more.add_buy();
  // Server& server = Server::getInstance();
  // int seq_num = server.getSeqNum();
  // apurchase->set_seqnum(seq_num);
  // apurchase->set_whnum(warehouse_id);

  // AProduct* aproduct = apurchase->add_things();
  // aproduct->set_id(product_id);
  // aproduct->set_count(quantity);
  // aproduct->set_description(description);

  // trySendMsgToWorld(buy_more, seq_num);
  purchaseMore(warehouse_id, product_id, description, quantity);
}

void send_to_user(std::string message, int client_connection_fd){
  Server& server = Server::getInstance();
  server.sendAllData(client_connection_fd, message.c_str(), message.length());
}

void send_to_user_no_stock(size_t product_id, std::string product_name, int client_connection_fd){
  stringstream ss;
  ss << "Sorry, currently we don't have enough product in your order.\n";
  ss << "Product id: " << product_id << "  Product_name: " << product_name << std::endl;
  send_to_user(ss.str(), client_connection_fd);
}

void handleOrder(const std::map<std::string, std::vector<std::string>> & headerMap, int client_connection_fd){
  try{
    Database& db = Database::getInstance();
    size_t user_id = get_user_id_order(headerMap);
    std::string user_name = get_user_name_order(headerMap);
    std::vector<std::pair<std::pair<size_t, std::string>, size_t>> products = get_products_order(headerMap);
    std::map<size_t, std::vector<std::pair<std::pair<size_t, std::string>, size_t>>> warehouse_products;
    bool all_enough = true;
    for (std::pair<std::pair<size_t, std::string>, size_t> product : products){
      size_t product_id = product.first.first;
      std::string product_name = product.first.second;
      size_t quantity = product.second;
      int warehouse_id = db.match_inventory(product_id, quantity);
      if (warehouse_id == -100){  // not enough stock for the current product
        // for now, just use product_name as its description and buy 10 more 
        send_to_world_buy_mroe(warehouse_id, product_id, product_name, 10);
        send_to_user_no_stock(product_id, product_name, client_connection_fd);
        all_enough = false;
        break;
      }
      if (warehouse_id >= 0){ // enough stock for the current product
        // add the current product to the map warehouse_products
        std::pair<std::pair<size_t, std::string>, size_t> product_info = std::make_pair(std::make_pair(product_id, product_name), quantity);\
        warehouse_products[warehouse_id].push_back(product_info);
      }
    }
    if (all_enough && !warehouse_products.empty()){
      for (item in map){
        generate order entry with package_id and status = "packing";
        send message to world: pack;
    //    send pickup request to UPS;
      }
    }

    
    
  }

  
} 

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
     const std::map<std::string, std::vector<std::string>> headerMap = customer_request.getHeaderMap();
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