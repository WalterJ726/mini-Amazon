#ifndef _HANDWORLD_HPP
#define _HANDWORLD_HPP
#include "Server.hpp"

void handleWorldResponse(AResponses& aresponses);
void purchaseMore(const int wh_id, const int p_id, const std::string p_name, const int p_num);
void processPurchaseMore(APurchaseMore& apurchasemore);
void processPacked(APacked& apacked);
void processLoaded(ALoaded& aloaded);
void trySendMsgToWorld(ACommands& ac, int seq_num);
void initProductsAmount();

#endif // _HANDWORLD_HPP