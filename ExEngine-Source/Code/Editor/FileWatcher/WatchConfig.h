#pragma once
#include <vector>
#include <string>
#include <chrono>
#include "../../Engine/JsonUtility/IJsonConvertable.h"

struct WatchConfig : public IJsonConvertable {
    std::vector<std::string> watchPaths;
    std::vector<std::string> fileExtensions;  // Empty = watch all
    std::vector<std::string> excludePatterns; // Regex patterns to exclude
    bool recursive = true;
    std::chrono::milliseconds debounceTime{100};
    bool watchHiddenFiles = false;
    
    // IJsonConvertable implementation
    nlohmann::json ToJson() override {
        nlohmann::json j;
        j["watchPaths"] = watchPaths;
        j["fileExtensions"] = fileExtensions;
        j["excludePatterns"] = excludePatterns;
        j["recursive"] = recursive;
        j["debounceTimeMs"] = debounceTime.count();
        j["watchHiddenFiles"] = watchHiddenFiles;
        return j;
    }
    
    void FromJson(const nlohmann::json& j) override {
        if (j.contains("watchPaths")) {
            watchPaths = j["watchPaths"].get<std::vector<std::string>>();
        }
        if (j.contains("fileExtensions")) {
            fileExtensions = j["fileExtensions"].get<std::vector<std::string>>();
        }
        if (j.contains("excludePatterns")) {
            excludePatterns = j["excludePatterns"].get<std::vector<std::string>>();
        }
        if (j.contains("recursive")) {
            recursive = j["recursive"];
        }
        if (j.contains("debounceTimeMs")) {
            int64_t ms = j["debounceTimeMs"];
            debounceTime = std::chrono::milliseconds(ms);
        }
        if (j.contains("watchHiddenFiles")) {
            watchHiddenFiles = j["watchHiddenFiles"];
        }
    }
};