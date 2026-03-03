#include "Server.h"
#include "ConnectionHandler.h"
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <fstream>
#include <iostream>
#include <csignal>

namespace gaijin
{

struct Server::Impl
{
    Impl(Storage& storage, Persistence& persistence, CommandHandler& commandHandler,
         const std::string& serverConfigPath)
        : aStorage(storage)
        , aPersistence(persistence)
        , aCommandHandler(commandHandler)
        , aServerConfigPath(serverConfigPath)
        , aPort(12345)
    {
        LoadConfig();
    }

    void LoadConfig()
    {
        std::ifstream f(aServerConfigPath);
        if (!f.is_open())
        {
            return;
        }
        std::string line;
        while (std::getline(f, line))
        {
            auto pos = line.find('=');
            if (pos == std::string::npos || pos == 0)
            {
                continue;
            }
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            if (key == "host")
            {
                aHost = value;
            }
            else if (key == "port")
            {
                try
                {
                    unsigned long port = std::stoul(value);
                    if (port > 65535)
                    {
                        port = 12345;
                    }
                    aPort = static_cast<uint16_t>(port);
                }
                catch (const std::exception&)
                {
                    aPort = 12345;
                }
            }
        }
        if (aHost.empty())
        {
            aHost = "0.0.0.0";
        }
    }

    void RunStatsThread()
    {
        while (!aStop)
        {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            if (aStop)
            {
                break;
            }
            size_t last5 = aCommandsLast5Sec.exchange(0);
            size_t total = aTotalCommands.load();
            std::cout << "Stats: total=" << total << ", last5sec=" << last5 << std::endl;
        }
    }

    void DoAccept()
    {
        aAcceptor->async_accept([this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
        {
            if (ec || aStop)
            {
                return;
            }
            ConnectionHandler handler(aCommandHandler, aTotalCommands, aCommandsLast5Sec);
            std::thread([handler, socket = std::move(socket)]() mutable
            {
                handler.Handle(std::move(socket));
            }).detach();
            DoAccept();
        });
    }

    void Stop()
    {
        aStop = true;
        if (aIoContext && aAcceptor)
        {
            boost::asio::post(*aIoContext, [this]()
            {
                if (aAcceptor)
                {
                    boost::system::error_code ec;
                    aAcceptor->close(ec);
                }
            });
        }
    }

    Storage& aStorage;
    Persistence& aPersistence;
    CommandHandler& aCommandHandler;
    std::string aServerConfigPath;
    std::string aHost;
    uint16_t aPort;
    std::atomic<bool> aStop{false};
    std::atomic<size_t> aTotalCommands{0};
    std::atomic<size_t> aCommandsLast5Sec{0};
    std::thread aStatsThread;

    std::unique_ptr<boost::asio::io_context> aIoContext;
    std::unique_ptr<boost::asio::ip::tcp::acceptor> aAcceptor;
    std::unique_ptr<boost::asio::signal_set> aSignals;
};

Server::Server(Storage& storage, Persistence& persistence, CommandHandler& commandHandler,
              const std::string& serverConfigPath)
    : aImpl(std::make_unique<Impl>(storage, persistence, commandHandler, serverConfigPath))
{
}

Server::~Server()
{
    Stop();
    if (aImpl && aImpl->aStatsThread.joinable())
    {
        aImpl->aStatsThread.join();
    }
}

void Server::Run()
{
    auto& impl = *aImpl;
    impl.aIoContext = std::make_unique<boost::asio::io_context>();
    impl.aAcceptor = std::make_unique<boost::asio::ip::tcp::acceptor>(
        *impl.aIoContext,
        boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address(impl.aHost), impl.aPort));
    impl.aSignals = std::make_unique<boost::asio::signal_set>(*impl.aIoContext, SIGINT, SIGTERM);

    impl.aSignals->async_wait([this](boost::system::error_code, int)
    {
        aImpl->Stop();
    });

    impl.aStatsThread = std::thread(&Impl::RunStatsThread, aImpl.get());

    std::cout << "Server listening on " << impl.aHost << ":" << impl.aPort << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;

    impl.DoAccept();
    impl.aIoContext->run();

    if (impl.aStatsThread.joinable())
    {
        impl.aStatsThread.join();
    }
}

void Server::Stop()
{
    if (aImpl)
    {
        aImpl->Stop();
    }
}

}  // namespace gaijin