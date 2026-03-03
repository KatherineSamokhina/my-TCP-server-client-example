#include "Server.h"
#include "ServerImpl.h"
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <csignal>
#include <iostream>

namespace gaijin
{

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