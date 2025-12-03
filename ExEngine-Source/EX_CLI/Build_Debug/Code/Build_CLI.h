#pragma once
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <string>

class Build_CLI{
    public:
    static void Build(){
        std::string bin = CLI_BIN_PATH;
        std::string cmmdToPath = "cd " + bin + " && cd ../../../ && ls";

        std::string cmmdToBuild = cmmdToPath + " && cmake --preset=engine-debug && cmake --build ./build/engine-debug";

        auto buildResult = system(cmmdToBuild.c_str());

        if(buildResult == 0){
            std::cout << "Build finished with success!";
        }
        else{
            std::cout << "Build failed!";
        }
    };
};