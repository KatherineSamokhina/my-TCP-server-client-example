#include "CommandHandler.h"
#include <sstream>
#include <algorithm>
#include <cctype>

namespace gaijin
{

CommandHandler::CommandHandler(Storage& storage, Persistence& persistence)
    : aStorage(storage)
    , aPersistence(persistence)
{
}

std::string CommandHandler::Handle(const std::string& command)
{
    if (command.empty())
    {
        return "ERROR=empty command\nreads=0\nwrites=0\n";
    }

    if (command.compare(0, 5, "$get ") == 0)
    {
        std::string key = command.substr(5);
        auto end = std::find_if(key.begin(), key.end(), [](char c) { return std::isspace(static_cast<unsigned char>(c)); });
        key.erase(end, key.end());
        return HandleGet(key);
    }

    if (command.compare(0, 5, "$set ") == 0)
    {
        std::string rest = command.substr(5);
        auto pos = rest.find('=');
        if (pos == std::string::npos || pos == 0)
        {
            return "ERROR=invalid set format\nreads=0\nwrites=0\n";
        }
        std::string key = rest.substr(0, pos);
        std::string value = rest.substr(pos + 1);
        auto endVal = std::find_if(value.begin(), value.end(), [](char c) { return std::isspace(static_cast<unsigned char>(c)); });
        value.erase(endVal, value.end());
        return HandleSet(key, value);
    }

    return "ERROR=unknown command\nreads=0\nwrites=0\n";
}

std::string CommandHandler::HandleGet(const std::string& key)
{
    auto value = aStorage.get(key);
    aStorage.IncrementReads(key);
    auto stats = aStorage.GetKeyStats(key);

    if (!value)
    {
        return FormatStats("ERROR=key not found", stats);
    }

    return FormatStats(key + "=" + *value, stats);
}

std::string CommandHandler::HandleSet(const std::string& key, const std::string& value)
{
    aStorage.set(key, value);
    aStorage.IncrementWrites(key);
    aPersistence.ScheduleFlush();

    auto stats = aStorage.GetKeyStats(key);
    return FormatStats("OK", stats);
}

std::string CommandHandler::FormatStats(const std::string& line, const Storage::KeyStats& stats)
{
    std::ostringstream oss;
    oss << line << "\nreads=" << stats.reads << "\nwrites=" << stats.writes << "\n";
    return oss.str();
}

}  // namespace gaijin