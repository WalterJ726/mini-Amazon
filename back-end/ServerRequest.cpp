#include "ServerRequest.hpp"


ServerRequest::ServerRequest(const std::string &rawRequest) : serverRequest(rawRequest){
    // requestTime = Time::getLocalUTC();
    hasError = false;
    try
    {
        parseStartLine();
        parseHeaderFields();
    }
    catch (const std::exception &e)
    {
        hasError = true;
        std::cout << "Malformed Request" << std::endl;
    }
}

void ServerRequest::parseStartLine(){
    size_t requestLineEnd = serverRequest.find("\n"); // TODO: there is no \r\n in request header
    actionLine = serverRequest.substr(0, requestLineEnd);
}

void ServerRequest::parseHeaderFields(){
    size_t requestLineEnd = serverRequest.find("\n");
    size_t headerEnd = serverRequest.find("\n\n");
    std::string headers = serverRequest.substr(requestLineEnd + 1, headerEnd - requestLineEnd - 1);
    size_t pos = 0;
    while (true){
        size_t end = headers.find("\n", pos);
        std::string line = headers.substr(pos, end - pos);
        pos = end + 1;
        size_t colon_pos = line.find(":");
        if (colon_pos != std::string::npos)
        {
            std::string key = line.substr(0, colon_pos);
            std::transform(key.begin(), key.end(), key.begin(),
                        [](unsigned char c)
                        { return std::tolower(c); });
            std::string value = line.substr(colon_pos + 1); // +2 to skip ": "
            headerMap[key].push_back(value);
        }
        if (end == std::string::npos)
        {
            break;
        }
    }
}

std::string ServerRequest::getAction() const{
    return actionLine;
}
std::string ServerRequest::getBindUpsName() const{
    return bindUpsName;
}

std::map<std::string, std::vector<std::string>> ServerRequest::getHeaderMap() const{
    return headerMap;
}