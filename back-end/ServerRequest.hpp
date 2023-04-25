#ifndef __SERVERREQUEST_HPP__
#define __SERVERREQUEST_HPP__
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <regex>
#include <algorithm>

class ServerRequest{
private:
    bool hasError;
    std::string serverRequest;
    std::string actionLine;
    std::map<std::string, std::vector<std::string>> headerMap;
    std::string bindUpsName;
    std::string requestTime;
public:
    ServerRequest(const std::string& rawRequest);
    std::string getAction() const;
    std::string getBindUpsName() const;
    std::map<std::string, std::vector<std::string>> getHeaderMap() const;
    
    // parse function
    void parseStartLine();
    void parseHeaderFields();

};


#endif