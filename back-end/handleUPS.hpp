#ifndef _HANDLEUPS_H
#define _HANDLEUPS_H
#include "handleProto.hpp"
#include "Server.hpp"

class HandleUPS{
    private:
        std::vector<UAbindUPSResponse> uabindreponses;
        std::vector<UAtruckArrived> uatruckArrs;
        std::vector<UAdelivered> uadelivereds;
        std::vector<UAchangeResp> uachangeResps;
        std::vector<int> seqNums;

    public:
        HandleUPS(const UAcommands& UAresponses);
        ~HandleUPS() {}
        void handleUPSResponse();
};
// parse reponse
void processbindUPSResponse(UAbindUPSResponse bindreponse);
void processUAtruckArrived(UAtruckArrived truckArr);
void processUAdelivered(UAdelivered delivered);
void processUAchangeResp(UAchangeResp changeResp);


// send msg to UPS
void sendUPSReqDelivery(const int ship_id);
void trySendMsgToUPS(AUcommands& ac, int seq_num);



#endif // _HANDLEUPS_H