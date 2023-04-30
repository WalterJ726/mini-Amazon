#ifndef _HANDWORLD_HPP
#define _HANDWORLD_HPP
#include "Server.hpp"

class HandleWorld{
    private:
        std::vector<APurchaseMore> apurchasemores;
        std::vector<APacked> apackeds;
        std::vector<ALoaded> aloadeds;
        std::vector<int> seqNums;

    public:
        HandleWorld(const AResponses & aresponses);
        ~HandleWorld() {}
        void handleWorldResponse();
};

void purchaseMore(const int wh_id, const int p_id, const std::string p_name, const int p_num);
void processPurchaseMore(APurchaseMore apurchasemore);
void processPacked(APacked apacked);
void processLoaded(ALoaded aloaded);
void trySendMsgToWorld(ACommands& ac, int seq_num);
void initProductsAmount();
bool checkHasHandled(int seqnum);

#endif // _HANDWORLD_HPP