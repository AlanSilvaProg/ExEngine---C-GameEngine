#pragma once
#include <string>
#include <unordered_map>

struct ProcessInfo{
    int id = -1;
    std::string description;
    float progress = 0.0f; // 0-100
};

// Tracks long-running background operations (script compilation, asset imports, etc.) so the editor
// can surface a single blocking progress window instead of leaving the UI looking unresponsive.
// Main-thread only: report progress from the same thread driving the main loop, the same assumption
// ScriptHotReloadManager::Poll() already makes when applying background work results.
class ProcessTracker{
private:
    static inline std::unordered_map<int, ProcessInfo> activeProcesses;
    static inline int nextProcessId = 0;
    static inline int currentProcessId = -1;
    static inline int completedInBatch = 0;
    static inline int totalInBatch = 0;

public:
    // Starts tracking a new process and returns the id to pass to SetProgress/EndProcess. Starting
    // one while every previous process has already ended begins a fresh n-of-total batch.
    static inline int BeginProcess(const std::string& description){
        if(activeProcesses.empty())
        {
            completedInBatch = 0;
            totalInBatch = 0;
        }

        const int id = nextProcessId++;
        totalInBatch++;
        currentProcessId = id;

        activeProcesses.emplace(id, ProcessInfo{id, description, 0.0f});
        return id;
    };

    static inline void EndProcess(const int processId){
        auto it = activeProcesses.find(processId);
        if(it == activeProcesses.end()) return;

        activeProcesses.erase(it);
        completedInBatch++;

        if(currentProcessId == processId)
        {
            currentProcessId = activeProcesses.empty() ? -1 : activeProcesses.begin()->first;
        }
    };

    static inline void SetProgress(const int processId, const float progressPercent){
        auto it = activeProcesses.find(processId);
        if(it != activeProcesses.end()) it->second.progress = progressPercent;
    };

    static inline bool HasActiveProcesses(){ return !activeProcesses.empty(); };
    static inline int GetCompletedInBatch(){ return completedInBatch; };
    static inline int GetTotalInBatch(){ return totalInBatch; };

    static inline const ProcessInfo& GetCurrentProcess(){
        static const ProcessInfo empty{};
        if(currentProcessId == -1) return empty;

        auto it = activeProcesses.find(currentProcessId);
        return it != activeProcesses.end() ? it->second : empty;
    };
};
