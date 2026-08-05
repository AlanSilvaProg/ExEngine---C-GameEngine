#pragma once
#include <string>
#include <vector>

struct ProcessResult{
    int exitCode = -1;
    std::string output; // combined stdout + stderr
    inline bool Succeeded() const { return exitCode == 0; };
};

// Runs an external command and captures its combined stdout/stderr output. Generalizes the
// popen() pattern already used by StackTrace.cpp (there only used to read atos/addr2line output)
// into a reusable helper for invoking the compiler on hot-reloaded scripts.
class ProcessRunner{
public:
    static ProcessResult Run(const std::vector<std::string>& argv);
};
