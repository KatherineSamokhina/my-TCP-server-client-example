#include "Storage.h"
#include <fstream>

namespace gaijin
{

Storage::Storage(const std::string& configPath) : aConfigPath(configPath)
{
    Load();
}

void Storage::Load()
{
    std::unique_lock lock(aMutex);
    aData.clear();
    std::ifstream f(aConfigPath);
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
        aData[std::move(key)] = std::move(value);
    }
}

std::optional<std::string> Storage::get(const std::string& key)
{
    std::shared_lock lock(aMutex);
    auto it = aData.find(key);
    if (it == aData.end())
    {
        return std::nullopt;
    }
    return it->second;
}

void Storage::set(const std::string& key, const std::string& value)
{
    std::unique_lock lock(aMutex);
    aData[key] = value;
}

Storage::KeyStats Storage::GetKeyStats(const std::string& key) const
{
    std::shared_lock lock(aMutex);
    auto it = aKeyStats.find(key);
    if (it == aKeyStats.end())
    {
        return {};
    }
    return it->second;
}

void Storage::IncrementReads(const std::string& key)
{
    std::unique_lock lock(aMutex);
    aKeyStats[key].reads++;
}

void Storage::IncrementWrites(const std::string& key)
{
    std::unique_lock lock(aMutex);
    aKeyStats[key].writes++;
}

Storage::Map Storage::Snapshot() const
{
    std::shared_lock lock(aMutex);
    return aData;
}

}  // namespace gaijin