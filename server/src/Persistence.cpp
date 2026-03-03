#include "Persistence.h"
#include <chrono>
#include <fstream>
#include <iostream>

namespace gaijin
{

Persistence::Persistence(Storage& storage, const std::string& configPath,
                         std::chrono::milliseconds interval)
    : aStorage(storage)
    , aConfigPath(configPath)
    , aInterval(interval)
{
    aThread = std::thread(&Persistence::Run, this);
}

Persistence::~Persistence()
{
    aStop = true;
    aCv.notify_one();
    if (aThread.joinable())
    {
        aThread.join();
    }
}

void Persistence::ScheduleFlush()
{
    {
        std::lock_guard lock(aMutex);
        aPending = true;
    }
    aCv.notify_one();
}

void Persistence::Run()
{
    while (!aStop)
    {
        std::unique_lock lock(aMutex);
        aCv.wait_for(lock, aInterval, [this]()
        {
            return aStop || aPending;
        });
        if (aStop)
        {
            break;
        }
        if (!aPending)
        {
            continue;
        }
        aPending = false;
        lock.unlock();

        auto data = aStorage.Snapshot();
        std::ofstream f(aConfigPath);
        if (f.is_open())
        {
            for (const auto& [k, v] : data)
            {
                f << k << '=' << v << '\n';
            }
        }
        else
        {
            std::cerr << "Persistence: failed to open " << aConfigPath << " for writing" << std::endl;
        }
    }
}

}  // namespace gaijin