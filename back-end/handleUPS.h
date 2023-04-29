#ifndef _HANDLEUPS_H
#define _HANDLEUPS_H
#include "handleProto.hpp"
#include "Server.hpp"

void handleUPSResponse(UAcommands UAresponses);

// parse reponse
void processbindUPSResponse(UAbindUPSResponse bindreponse);
void processUAtruckArrived(UAtruckArrived truckArr);
void processUAdelivered(UAdelivered delivered);
void processUAchangeResp(UAchangeResp changeResp);


// send msg to UPS
void sendUPSReqDelivery(const int ship_id);
void trySendMsgToUPS(AUcommands& ac, int seq_num);



#endif // _HANDLEUPS_H