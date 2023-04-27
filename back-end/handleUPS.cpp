#include "handleUPS.h"

void handleUPSResponse(UAcommands& UAresponses){
      Server& server = Server::getInstance();
      for (int i = 0; i < UAresponses.acks_size(); i ++ ){
        std::cout << "UAresponses.acks(i) is " << UAresponses.acks(i) << std::endl;
        server.ups_finished_SeqNum_set.insert(UAresponses.acks(i));
      }

      // start to parse UAbindUPSResponse
      AUcommands UAbindUPSResponse_ack;
      std::cout << "start to parse UAbindUPSResponse" << std::endl;
      for (int i = 0; i < UAresponses.bindupsresponse_size(); i ++ ){
          UAbindUPSResponse bindreponse = UAresponses.bindupsresponse(i);
          int seqnum = bindreponse.seqnum();
          std::cout << "bindreponse.seqnum() is " << seqnum << std::endl;
          if (server.ups_finished_SeqNum_set.find(seqnum) != server.ups_finished_SeqNum_set.end()){
              continue;
          }
          processbindUPSResponse(bindreponse);
          UAbindUPSResponse_ack.add_acks(seqnum);
      }
      server.A2U_send_queue.push(UAbindUPSResponse_ack);

      std::cout << "start to parse UAtruckArrived" << std::endl;
      // start to parse UAtruckArrived
      AUcommands UAtruckArrived_ack;
      for (int i = 0; i < UAresponses.truckarr_size(); i ++ ){
        UAtruckArrived truckArr = UAresponses.truckarr(i);
        int seqnum = truckArr.seqnum();
        std::cout << "truckArr.seqnum() is " << seqnum << std::endl;
        if (server.ups_finished_SeqNum_set.find(seqnum) != server.ups_finished_SeqNum_set.end()){
          continue;
        }
        processUAtruckArrived(truckArr);
        UAtruckArrived_ack.add_acks(seqnum);
      }
      server.A2U_send_queue.push(UAtruckArrived_ack);

      AUcommands UAstatus_ack;
      std::cout << "start to parse UAstatus" << std::endl;
      // start to parse APacked
      for (int i = 0; i < UAresponses.status_size(); i ++ ){
        UAstatus status = UAresponses.status(i);
        int seqnum = status.seqnum();
        if (server.ups_finished_SeqNum_set.find(seqnum) != server.ups_finished_SeqNum_set.end()){
          continue;
        }
        processUAstatus(status);
        UAstatus_ack.add_acks(seqnum);
      }
      server.A2U_send_queue.push(UAstatus_ack);

      AUcommands UAdelivered_ack;
      std::cout << "start to parse UAdelivered" << std::endl;
      // start to parse UAdelivered
      for (int i = 0; i < UAresponses.delivered_size(); i ++ ){
        UAdelivered delivered = UAresponses.delivered(i);
        int seqnum = delivered.seqnum();
        if (server.ups_finished_SeqNum_set.find(seqnum) != server.ups_finished_SeqNum_set.end()){
          continue;
        }
        processUAdelivered(delivered);
        UAdelivered_ack.add_acks(seqnum);
      }
      server.A2U_send_queue.push(UAdelivered_ack);


      AUcommands UAchangeResp_ack;
      std::cout << "start to parse UAchangeResp" << std::endl;
      // start to parse UAchangeResp
      for (int i = 0; i < UAresponses.changeresp_size(); i ++ ){
        UAchangeResp changeResp = UAresponses.changeresp(i);
        int seqnum = changeResp.seqnum();
        if (server.ups_finished_SeqNum_set.find(seqnum) != server.ups_finished_SeqNum_set.end()){
          continue;
        }
        processUAchangeResp(changeResp);
        UAchangeResp_ack.add_acks(seqnum);
      }
      server.A2U_send_queue.push(UAchangeResp_ack);

}

void processbindUPSResponse(UAbindUPSResponse& bindreponse){

}

void processUAtruckArrived(UAtruckArrived& truckArr){
  // notify the world start to loaded
  ACommands acommand;
  APutOnTruck* load = acommand.add_load();
  Server& server = Server::getInstance();
  int seq_num = server.getSeqNum();
  load->set_seqnum(seq_num);
  load->set_whnum(truckArr.whid());
  load->set_truckid(truckArr.truckid());
  load->set_shipid(truckArr.shipid());
  trySendMsgToWorld(acommand, seq_num);
}

void processUAstatus(UAstatus& uastatus){
  // update the status of the specific package to the status from response
  // shipID
  Database& db = Database::getInstance();
  std::string status = uastatus.status();
  // db.update_package_status(ship_id, status);
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

void trySendMsgToUPS(AUcommands& ac, int seq_num){
  // periodically thread, for at least once
  std::cout << "try to send MSG to UPS, for at least once: " <<  std::endl;
  Server& server = Server::getInstance();
  while (1){
      // std::cout << "start to periodically thread" <<  std::endl;
      server.A2U_send_queue.push(ac);
      std::this_thread::sleep_for(std::chrono::milliseconds(1000));
      if (server.ups_finished_SeqNum_set.find(seq_num) != server.ups_finished_SeqNum_set.end()){
        break;
      }
  }
  std::cout << "finished periodically thread in sending MSG to UPS" <<  std::endl;
}
