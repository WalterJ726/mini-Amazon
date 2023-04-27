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

std::pair<int, int> get_dest_x_y_order(const std::map<std::string, std::vector<std::string>> & headerMap){
  try{
    int dest_x = std::stoi(headerMap.at("dest_x").at(0));
    int dest_y = std::stoi(headerMap.at("dest_y").at(0));
    return std::pair<int,int>(dest_x, dest_y);
  }
  catch (const std::exception & e) {
    std::cerr << e.what() << '\n';
    std::cout << "dest_x or dest_y is invalid when using get_dest_x_y_order to parse it" << std::endl;
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

void send_to_world_buy_more(size_t warehouse_id, size_t product_id, std::string description, size_t quantity){
  purchaseMore(warehouse_id, product_id, description, quantity);
}

void send_to_world_pack(long package_id, const std::pair<size_t, std::vector<std::pair<std::pair<size_t, std::string>, size_t>>> & warehouse_products){
  ACommands pack;
  APack* apack = pack.add_topack();
  Server& server = Server::getInstance();
  int seq_num = server.getSeqNum();
  size_t warehouse_id = warehouse_products.first;
  apack->set_seqnum(seq_num);
  apack->set_whnum(warehouse_id);
  apack->set_shipid(package_id);

  for (std::vector<std::pair<std::pair<size_t, std::string>, size_t>>::const_iterator curr_product = warehouse_products.second.begin(); curr_product != warehouse_products.second.end(); ++curr_product){
    size_t product_id = curr_product->first.first;
    size_t quantity = curr_product->second;
    const std::string product_name = curr_product->first.second;
    AProduct* aproduct = apack->add_things();
    aproduct->set_id(product_id);
    aproduct->set_count(quantity);
    aproduct->set_description(product_name);
  }

  trySendMsgToWorld(pack, seq_num);
}

void send_to_ups_pack(long package_id, const std::pair<int, int> & dest_x_y, const std::pair<size_t, std::vector<std::pair<std::pair<size_t, std::string>, size_t>>> & warehouse_products){
  AUcommands aucommands;
  AUreqPickup* pickup = aucommands.add_pickup();
  Server& server = Server::getInstance();
  int seq_num = server.getSeqNum();
  size_t warehouse_id = warehouse_products.first;
  pickup->set_seqnum(seq_num);
  pickup->set_whid(warehouse_id);
  pickup->set_shipid(package_id);
  pickup->set_destinationx(dest_x_y.first);
  pickup->set_destinationy(dest_x_y.second);
  for (std::vector<std::pair<std::pair<size_t, std::string>, size_t>>::const_iterator curr_product = warehouse_products.second.begin(); curr_product != warehouse_products.second.end(); ++curr_product){
    size_t product_id = curr_product->first.first;
    size_t quantity = curr_product->second;
    const std::string product_name = curr_product->first.second;
    AProduct* aproduct = pickup->add_products();
    aproduct->set_id(product_id);
    aproduct->set_count(quantity);
    aproduct->set_description(product_name);
  }
  trySendMsgToUPS(aucommands, seq_num);
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

void generate_insert_order_package(size_t user_id, const long order_num, const long package_id, const std::pair<int, int> & dest_x_y, const std::pair<size_t, std::vector<std::pair<std::pair<size_t, std::string>, size_t>>> & warehouse_products){
  Database& db = Database::getInstance();
  int dest_x = dest_x_y.first;
  int dest_y = dest_x_y.second;
  size_t warehouse_id = warehouse_products.first;
  db.insert_package(package_id, user_id, warehouse_id, dest_x, dest_y);
  for (std::vector<std::pair<std::pair<size_t, std::string>, size_t>>::const_iterator curr_product = warehouse_products.second.begin(); curr_product != warehouse_products.second.end(); ++curr_product){
    size_t product_id = curr_product->first.first;
    size_t quantity = curr_product->second;
    const std::string order_status = "packing";
    db.insert_order(order_num, product_id, user_id, quantity, order_status, package_id, time_t(NULL));
  }
}

void handleOrder(const std::map<std::string, std::vector<std::string>> & headerMap, int client_connection_fd){
  try{
    Database& db = Database::getInstance();
    size_t user_id = get_user_id_order(headerMap);
    std::string user_name = get_user_name_order(headerMap);
    std::pair<int, int> dest_x_y = get_dest_x_y_order(headerMap);
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
        send_to_world_buy_more(warehouse_id, product_id, product_name, 10);
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
      Server& server = Server::getInstance();
      long package_id = server.getPackageID();
      long order_num = server.getOrderNum();
      // send message to user: order placed successfully!
      send_to_user("Order placed successfully!", client_connection_fd);
      for(std::map<size_t, std::vector<std::pair<std::pair<size_t, std::string>, size_t>>>::const_iterator curr_warehouse = warehouse_products.begin(); curr_warehouse != warehouse_products.end(); ++curr_warehouse){
        generate_insert_order_package(user_id, order_num, package_id, dest_x_y, *curr_warehouse);
        send_to_world_pack(package_id, *curr_warehouse);
        //    send pickup request to UPS;
        send_to_ups_pack(package_id, dest_x_y, *curr_warehouse);
      }
    }
  }
  catch(const std::exception & e) {
    std::cerr << e.what() << '\n';
    std::cout << "error when handling order from user" << std::endl;
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
    std::cout << recv_str_front_end << std::endl;
    ServerRequest customer_request(recv_str_front_end);
    std::string action = customer_request.getAction();
    if (action == "bind"){
      // start to process bind
    } else {
      // start to process order
     const std::map<std::string, std::vector<std::string>> headerMap = customer_request.getHeaderMap();
      for (auto it = headerMap.begin(); it != headerMap.end(); ++it) {
          std::cout << it->first << std::endl;
          for (int i = 0; i < it->second.size(); i ++ ){
              std::cout << it->second[i] << " ";
          }
          std::cout << std::endl;
      }
      handleOrder(headerMap, client_connection_fd); // create a new thread to handle customer order
    }
    close(client_connection_fd);
  }
}