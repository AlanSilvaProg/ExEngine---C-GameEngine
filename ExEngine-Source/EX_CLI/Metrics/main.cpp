#include "Code/Metrics_CLI.h"

int main(int argc, char* argv[]){
    bool includeThirdParty = true;
    
    // Check for --no-thirdparty flag
    for(int i = 1; i < argc; i++){
        if(std::string(argv[i]) == "--no-thirdparty"){
            includeThirdParty = false;
            break;
        }
    }
    
    Metrics_CLI::ShowMetrics(includeThirdParty);    

    return 0;
}