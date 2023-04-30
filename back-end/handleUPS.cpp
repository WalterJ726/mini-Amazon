#include "handleUPS.hpp"

HandleUPS::HandleUPS(const UAcommands& UAresponses){
  for (int i = 0; i < UAresponses.bindupsresponse_size(); i++) {
    uabindreponses.push_back(std::move(UAresponses.bindupsresponse(i)));
    seqNums.push_back(UAresponses.bindupsresponse(i).seqnum());
  }

  for (int i = 0; i < UAresponses.truckarr_size(); i++) {
    uatruckArrs.push_back(std::move(UAresponses.truckarr(i)));
    seqNums.push_back(UAresponses.truckarr(i).seqnum());
  }

  for (int i = 0; i < UAresponses.delivered_size(); i++) {
    uadelivereds.push_back(std::move(UAresponses.delivered(i)));
    seqNums.push_back(UAresponses.delivered(i).seqnum());
  }

  for (int i = 0; i < UAresponses.changeresp_size(); i++) {
    uachangeResps.push_back(std::move(UAresponses.changeresp(i)));
    seqNums.push_back(UAresponses.changeresp(i).seqnum());
  }
  
  Server& server = Server::getInstance();
  // record acks from world
  for (int i = 0; i < UAresponses.acks_size(); i ++ ){
    std::cout << "UAresponses.acks(i) is " << UAresponses.acks(i) << std::endl;
    server.global_finished_SeqNum_set.insert(UAresponses.acks(i));
  }
}

bool checkUPSHasHandled(int seqnum){
  Server& server = Server::getInstance();
  std::cout << "seqnum() is " << seqnum << std::endl;
  if (server.ups_finished_SeqNum_set.find(seqnum) != server.ups_finished_SeqNum_set.end()){
    return true;
  }
  server.ups_finished_SeqNum_set.insert(seqnum);
  return false;
}

void HandleUPS::handleUPSResponse(){
      Server& server = Server::getInstance();
      // ACK responses to world.
      AUcommands all_acks;
      for (size_t i = 0; i < seqNums.size(); i++) {
        all_acks.add_acks(i);
        all_acks.set_acks(i, seqNums[i]);
      }
      server.A2U_send_queue.push(all_acks);

      // start to parse UAbindUPSResponse
      for (size_t i = 0; i < uabindreponses.size(); i ++ ){
          UAbindUPSResponse bindreponse = uabindreponses[i];
          int seqnum = bindreponse.seqnum();
          if (!checkUPSHasHandled(seqnum)){
            std::cout << "start to parse UAbindUPSResponse" << std::endl;
            processbindUPSResponse(bindreponse);
          }
      }

      // start to parse UAtruckArrived
      for (size_t i = 0; i < uatruckArrs.size(); i ++ ){
        UAtruckArrived truckArr = uatruckArrs[i];
        int seqnum = truckArr.seqnum();
        if (!checkUPSHasHandled(seqnum)){
          std::cout << "start to parse UAtruckArrived" << std::endl;
          processUAtruckArrived(truckArr);
        }
      }

      // start to parse UAdelivered
      for (size_t i = 0; i < uadelivereds.size(); i ++ ){
        UAdelivered delivered = uadelivereds[i];
        int seqnum = delivered.seqnum();
        if (!checkUPSHasHandled(seqnum)){
          std::cout << "start to parse UAdelivered" << std::endl;
          processUAdelivered(delivered);
        }
      }

      // start to parse UAchangeResp
      for (size_t i = 0; i < uachangeResps.size(); i ++ ){
        UAchangeResp changeResp = uachangeResps[i];
        int seqnum = changeResp.seqnum();
        if (!checkUPSHasHandled(seqnum)){
          std::cout << "start to parse UAchangeResp" << std::endl;
          processUAchangeResp(changeResp);
        }
      }

}

void processbindUPSResponse(UAbindUPSResponse bindreponse){
  Database& db = Database::getInstance();
  bool isSuccess = bindreponse.status();
  std::string status_str = isSuccess == true ? "bind Success" : "bind failed";
  int user_id = bindreponse.ownerid();
  int ups_id = bindreponse.upsid();
  db.update_bind_status(user_id, ups_id, status_str);
}

void processUAtruckArrived(UAtruckArrived truckArr){
  // check whether the package status is packed 
  Database& db = Database::getInstance();
  std::string check_status = "packing";
  while (1){
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    if (!db.check_package_status(truckArr.shipid(), check_status)){
      // status is packed
      break;
    }
  }
  // notify the world start to loaded
  ACommands acommand;
  APutOnTruck* load = acommand.add_load();
  Server& server = Server::getInstance();
  int seq_num = server.getSeqNum();
  load->set_seqnum(seq_num);
  load->set_whnum(truckArr.whid());
  load->set_truckid(truckArr.truckid());
  load->set_shipid(truckArr.shipid());

  // change status to loading
  trySendMsgToWorld(acommand, seq_num);
  std::string status = "loading";
  db.update_package_status(truckArr.shipid(), status);
}

void processUAdelivered(UAdelivered delivered){
  // update the status of the specific package to delivered
  int ship_id = delivered.shipid();
  Database& db = Database::getInstance();
  std::string status = "delivered";
  db.update_package_status(ship_id, status);
}

void processUAchangeResp(UAchangeResp changeResp){
  // if status == true, then change the destination of the package

  // if status == false, then do nothing of the destination
}

void sendUPSReqDelivery(const int ship_id){
  AUcommands aucommand;
  AUreqDelivery* reqDelivery = aucommand.add_delivery();
  Server& server = Server::getInstance();
  int seq_num = server.getSeqNum();
  reqDelivery->set_seqnum(seq_num);
  reqDelivery->set_shipid(ship_id);
  trySendMsgToUPS(aucommand, seq_num);

  // change the package status to delivering
  Database& db = Database::getInstance();
  std::string status = "delivering";
  db.update_package_status(ship_id, status);
}

void trySendMsgToUPS(AUcommands& ac, int seq_num){
  // periodically thread, for at least once
  std::cout << "try to send MSG to UPS, for at least once: " <<  std::endl;
  Server& server = Server::getInstance();
  while (1){
      server.A2U_send_queue.push(ac);
      std::cout << ac.DebugString() << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      if (server.global_finished_SeqNum_set.find(seq_num) != server.global_finished_SeqNum_set.end()){
        break;
      }
  }
  std::cout << "finished periodically thread in sending MSG to UPS" <<  std::endl;
}
