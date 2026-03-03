#pragma once

#include <string>
#include <unordered_map>
#include <shared_mutex>
#include <optional>
#include <utility>

namespace gaijin
{

class Storage
{
public:
    struct KeyStats
    {
        size_t reads = 0;
        size_t writes = 0;
    };

    using Map = std::unordered_map<std::string, std::string>;
    using StatsMap = std::unordered_map<std::string, KeyStats>;

    explicit Storage(const std::string& configPath);

    std::optional<std::string> get(const std::string& key);
    void set(const std::string& key, const std::string& value);

    KeyStats GetKeyStats(const std::string& key) const;
    void IncrementReads(const std::string& key);
    void IncrementWrites(const std::string& key);

    Map Snapshot() const;

private:
    void Load();

    std::string aConfigPath;
    Map aData;
    mutable StatsMap aKeyStats;
    mutable std::shared_mutex aMutex;
};

}  // namespace gaijin