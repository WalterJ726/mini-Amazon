#include "handleUPS.h"

void handleUPSResponse(UAcommands& UAresponses){
      Server& server = Server::getInstance();
      std::cout << UAresponses.DebugString() << std::endl;
      for (int i = 0; i < UAresponses.acks_size(); i ++ ){
        std::cout << "UAresponses.acks(i) is " << UAresponses.acks(i) << std::endl;
        server.ups_finished_SeqNum_set.insert(UAresponses.acks(i));
      }

      // start to parse UAbindUPSResponse
      for (int i = 0; i < UAresponses.bindupsresponse_size(); i ++ ){
          AUcommands UAbindUPSResponse_ack;
          UAbindUPSResponse bindreponse = UAresponses.bindupsresponse(i);
          int seqnum = bindreponse.seqnum();
          std::cout << "bindreponse.seqnum() is " << seqnum << std::endl;
          if (server.ups_finished_SeqNum_set.find(seqnum) != server.ups_finished_SeqNum_set.end()){
              continue;
          }
          std::cout << "start to parse UAbindUPSResponse" << std::endl;
          UAbindUPSResponse_ack.add_acks(seqnum);
          server.A2U_send_queue.push(UAbindUPSResponse_ack);
          processbindUPSResponse(bindreponse);
      }

      // start to parse UAtruckArrived
      for (int i = 0; i < UAresponses.truckarr_size(); i ++ ){
        AUcommands UAtruckArrived_ack;
        UAtruckArrived truckArr = UAresponses.truckarr(i);
        int seqnum = truckArr.seqnum();
        std::cout << "truckArr.seqnum() is " << seqnum << std::endl;
        if (server.ups_finished_SeqNum_set.find(seqnum) != server.ups_finished_SeqNum_set.end()){
          continue;
        }
        std::cout << "start to parse UAtruckArrived" << std::endl;
        UAtruckArrived_ack.add_acks(seqnum);
        server.A2U_send_queue.push(UAtruckArrived_ack);
        processUAtruckArrived(truckArr);
      }

      // start to parse UAdelivered
      for (int i = 0; i < UAresponses.delivered_size(); i ++ ){
        AUcommands UAdelivered_ack;
        UAdelivered delivered = UAresponses.delivered(i);
        int seqnum = delivered.seqnum();
        if (server.ups_finished_SeqNum_set.find(seqnum) != server.ups_finished_SeqNum_set.end()){
          continue;
        }
        std::cout << "start to parse UAdelivered" << std::endl;
        UAdelivered_ack.add_acks(seqnum);
        server.A2U_send_queue.push(UAdelivered_ack);
        processUAdelivered(delivered);
      }

      // start to parse UAchangeResp
      for (int i = 0; i < UAresponses.changeresp_size(); i ++ ){
        AUcommands UAchangeResp_ack;
        UAchangeResp changeResp = UAresponses.changeresp(i);
        int seqnum = changeResp.seqnum();
        if (server.ups_finished_SeqNum_set.find(seqnum) != server.ups_finished_SeqNum_set.end()){
          continue;
        }
        std::cout << "start to parse UAchangeResp" << std::endl;
        UAchangeResp_ack.add_acks(seqnum);
        server.A2U_send_queue.push(UAchangeResp_ack);
        processUAchangeResp(changeResp);
      }

}

void processbindUPSResponse(UAbindUPSResponse& bindreponse){

}

void processUAtruckArrived(UAtruckArrived& truckArr){
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
  std::string status = "loading";
  db.update_package_status(truckArr.shipid(), status);
  trySendMsgToWorld(acommand, seq_num);
}


void processUAdelivered(UAdelivered& delivered){
  // update the status of the specific package to delivered
  int ship_id = delivered.shipid();
  Database& db = Database::getInstance();
  std::string status = "delivered";
  db.update_package_status(ship_id, status);
}

void processUAchangeResp(UAchangeResp& changeResp){
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
      // std::cout << "start to periodically thread" <<  std::endl;
      server.A2U_send_queue.push(ac);
      std::cout << ac.DebugString() << std::endl;
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      if (server.ups_finished_SeqNum_set.find(seq_num) != server.ups_finished_SeqNum_set.end()){
        break;
      }
  }
  std::cout << "finished periodically thread in sending MSG to UPS" <<  std::endl;
}
