#include "Storage.h"
#include "Persistence.h"
#include "CommandHandler.h"
#include "Server.h"
#include <iostream>
#include <chrono>

int main()
{
    try
    {
        std::string configPath = "config.txt";
        std::string serverConfigPath = "server_config.txt";

        gaijin::Storage storage(configPath);
        gaijin::Persistence persistence(storage, configPath, std::chrono::milliseconds(1000));
        gaijin::CommandHandler commandHandler(storage, persistence);
        gaijin::Server server(storage, persistence, commandHandler, serverConfigPath);

        server.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}