#include "ProcessRunner.h"

#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#else
#include <sys/wait.h>
#endif

static std::string QuoteArg(const std::string& arg){
    return "\"" + arg + "\"";
};

ProcessResult ProcessRunner::Run(const std::vector<std::string>& argv){
    ProcessResult result;

    if(argv.empty())
    {
        result.output = "ProcessRunner::Run called with an empty command";
        return result;
    }

    std::string command;
    for(const auto& arg : argv)
    {
        if(!command.empty()) command += " ";
        command += QuoteArg(arg);
    }
    command += " 2>&1"; // merge stderr into stdout so compiler errors are captured too

    FILE* pipe = popen(command.c_str(), "r");
    if(pipe == nullptr)
    {
        result.output = "Failed to start process: " + command;
        return result;
    }

    char buffer[512];
    while(fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        result.output += buffer;
    }

    int status = pclose(pipe);

#ifdef _WIN32
    result.exitCode = status;
#else
    result.exitCode = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
#endif

    return result;
};
