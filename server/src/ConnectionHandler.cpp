#include "ConnectionHandler.h"

namespace gaijin
{

ConnectionHandler::ConnectionHandler(CommandHandler& commandHandler,
                                     std::atomic<size_t>& totalCommands,
                                     std::atomic<size_t>& commandsLast5Sec)
    : aCommandHandler(commandHandler)
    , aTotalCommands(totalCommands)
    , aCommandsLast5Sec(commandsLast5Sec)
{
}

void ConnectionHandler::Handle(boost::asio::ip::tcp::socket socket)
{
    try
    {
        boost::asio::streambuf buf;
        std::istream is(&buf);
        std::string line;

        while (true)
        {
            boost::system::error_code ec;
            size_t n = boost::asio::read_until(socket, buf, '\n', ec);
            if (ec || n == 0)
            {
                break;
            }
            std::getline(is, line);
            if (!line.empty() && line.back() == '\r')
            {
                line.pop_back();
            }

            std::string response = aCommandHandler.Handle(line);
            aTotalCommands++;
            aCommandsLast5Sec++;

            boost::asio::write(socket, boost::asio::buffer(response));
        }
    }
    catch (const std::exception&)
    {
        // Connection closed or error
    }
}

}  // namespace gaijin