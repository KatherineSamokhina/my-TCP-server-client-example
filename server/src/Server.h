#pragma once

#include "CommandHandler.h"
#include <string>
#include <memory>

namespace gaijin
{

class Server
{
public:
    Server(Storage& storage, Persistence& persistence, CommandHandler& commandHandler,
           const std::string& serverConfigPath);
    ~Server();

    void Run();
    void Stop();

private:
    struct Impl;
    std::unique_ptr<Impl> aImpl;
};

}  // namespace gaijin