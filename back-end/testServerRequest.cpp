#include "ServerRequest.hpp"

int main(){
    const char* msg = "order\n"
        "product_id:1\n"
        "user_name:112233\n"
        "product_name:apple\n\n";
    std::string request_test = std::string(msg, strlen(msg));
    ServerRequest newServerRequest = ServerRequest(request_test);
    // start to verify http request
    std::cout << newServerRequest.getAction() << std::endl;
    std::map<std::string, std::vector<std::string>> headerMap = newServerRequest.getHeaderMap();
    for (auto it = headerMap.begin(); it != headerMap.end(); ++it) {
        std::cout << it->first << std::endl;
        for (int i = 0; i < it->second.size(); i ++ ){
            std::cout << it->second[i];
        }
        std::cout << std::endl;
    }
    return 0;
}