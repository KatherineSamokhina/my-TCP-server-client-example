#pragma once

#include "CommandHandler.h"
#include <atomic>
#include <boost/asio.hpp>

namespace gaijin
{

class ConnectionHandler
{
public:
    ConnectionHandler(CommandHandler& commandHandler,
                      std::atomic<size_t>& totalCommands,
                      std::atomic<size_t>& commandsLast5Sec);

    void Handle(boost::asio::ip::tcp::socket socket);

private:
    CommandHandler& aCommandHandler;
    std::atomic<size_t>& aTotalCommands;
    std::atomic<size_t>& aCommandsLast5Sec;
};

}  // namespace gaijin