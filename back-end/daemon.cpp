#include "Server.hpp"

int main(int argc, char** args)
{

    try
    {
        Server& server_daemon = Server::getInstance();
        server_daemon.ups_host = std::string(args[1]);
        if (server_daemon.getErrorSign()){
            std::cout << "fail to initialize server" << std::endl;
        }
        server_daemon.startRun();
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
        std::cout << "fail to initialize server" << std::endl;
    }
    return EXIT_SUCCESS;
}
