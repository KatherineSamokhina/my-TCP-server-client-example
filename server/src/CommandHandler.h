#pragma once

#include "Storage.h"
#include "Persistence.h"
#include <string>

namespace gaijin
{

class CommandHandler
{
public:
    CommandHandler(Storage& storage, Persistence& persistence);

    std::string Handle(const std::string& command);

private:
    std::string HandleGet(const std::string& key);
    std::string HandleSet(const std::string& key, const std::string& value);
    std::string FormatStats(const std::string& line, const Storage::KeyStats& stats);

    Storage& aStorage;
    Persistence& aPersistence;
};

}  // namespace gaijin