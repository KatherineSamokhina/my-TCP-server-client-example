#pragma once

#include "Storage.h"
#include <string>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <chrono>

namespace gaijin
{

class Persistence
{
public:
    Persistence(Storage& storage, const std::string& configPath,
                std::chrono::milliseconds interval);
    ~Persistence();

    void ScheduleFlush();

private:
    void Run();

    Storage& aStorage;
    std::string aConfigPath;
    std::chrono::milliseconds aInterval;
    std::thread aThread;
    std::atomic<bool> aStop{false};
    std::condition_variable aCv;
    std::mutex aMutex;
    bool aPending{false};
};

}  // namespace gaijin