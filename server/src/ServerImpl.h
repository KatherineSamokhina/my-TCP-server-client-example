#pragma once

#include "Server.h"
#include "ConnectionHandler.h"
#include <boost/asio.hpp>
#include <boost/asio/signal_set.hpp>
#include <atomic>
#include <chrono>
#include <csignal>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

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

}  // namespace gaijin
