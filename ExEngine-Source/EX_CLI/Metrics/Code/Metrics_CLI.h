#pragma once
#include <filesystem>
#include <iostream>
#include <cstdlib>
#include <stdlib.h>
#include <string>
#include <sstream>

class Metrics_CLI{
    public:
    static void ShowMetrics(bool includeThirdParty = true){
        std::string bin = CLI_BIN_PATH;
        std::string basePath = bin + "/../../../";
        
        std::cout << "\n=== MÉTRICAS DO PROJETO ExEngine ===\n\n";
        
        if(includeThirdParty){
            ShowFullMetrics(basePath);
        } else {
            ShowMetricsWithoutThirdParty(basePath);
        }
    }
    
    private:
    static void ShowFullMetrics(const std::string& basePath){
        std::cout << "📁 Arquivos por tipo (incluindo ThirdParty):\n";
        
        // Count .h files
        std::string countH = "find " + basePath + " -name \"*.h\" | wc -l";
        int hFiles = ExecuteAndGetNumber(countH);
        
        // Count .cpp files  
        std::string countCpp = "find " + basePath + " -name \"*.cpp\" | wc -l";
        int cppFiles = ExecuteAndGetNumber(countCpp);
        
        // Count .hpp files
        std::string countHpp = "find " + basePath + " -name \"*.hpp\" | wc -l";
        int hppFiles = ExecuteAndGetNumber(countHpp);
        
        std::cout << "   .h files:   " << hFiles << "\n";
        std::cout << "   .cpp files: " << cppFiles << "\n";
        std::cout << "   .hpp files: " << hppFiles << "\n";
        std::cout << "\n📊 Total de arquivos de código geral: " << (hFiles + cppFiles + hppFiles) << "\n\n";

        // Count lines
        std::string countLines = "find " + basePath + " -name \"*.h\" -o -name \"*.cpp\" -o -name \"*.hpp\" | while read file; do wc -l \"$file\" 2>/dev/null; done | awk '{sum += $1} END {print sum}'";
        int totalLines = ExecuteAndGetNumber(countLines);

        std::cout << "📏 Total de linhas de código geral: " << totalLines << "\n\n";

        // Count files/lines restricted to Engine/Editor
        std::string countEngineEditorFiles = "find " + basePath + "Code/Engine " + basePath + "Code/Editor -name \"*.h\" -o -name \"*.cpp\" -o -name \"*.hpp\" 2>/dev/null | wc -l";
        int engineEditorFiles = ExecuteAndGetNumber(countEngineEditorFiles);

        std::cout << "📊 Total de arquivos de código Engine/Editor: " << engineEditorFiles << "\n\n";

        std::string countEngineEditorLines = "find " + basePath + "Code/Engine " + basePath + "Code/Editor -name \"*.h\" -o -name \"*.cpp\" -o -name \"*.hpp\" 2>/dev/null | while read file; do wc -l \"$file\" 2>/dev/null; done | awk '{sum += $1} END {print sum}'";
        int engineEditorLines = ExecuteAndGetNumber(countEngineEditorLines);

        std::cout << "📏 Total de linhas de código Engine/Editor: " << engineEditorLines << "\n\n";

        ShowDirectoryBreakdown(basePath);
    }
    
    static void ShowMetricsWithoutThirdParty(const std::string& basePath){
        std::cout << "📁 Arquivos por tipo (excluindo ThirdParty):\n";
        
        // Count files excluding ThirdParty
        std::string countH = "find " + basePath + " -name \"*.h\" -not -path \"*/ThirdParty/*\" | wc -l";
        int hFiles = ExecuteAndGetNumber(countH);
        
        std::string countCpp = "find " + basePath + " -name \"*.cpp\" -not -path \"*/ThirdParty/*\" | wc -l";
        int cppFiles = ExecuteAndGetNumber(countCpp);
        
        std::string countHpp = "find " + basePath + " -name \"*.hpp\" -not -path \"*/ThirdParty/*\" | wc -l";
        int hppFiles = ExecuteAndGetNumber(countHpp);
        
        std::cout << "   .h files:   " << hFiles << "\n";
        std::cout << "   .cpp files: " << cppFiles << "\n";
        std::cout << "   .hpp files: " << hppFiles << "\n";
        std::cout << "\n📊 Total de arquivos de código: " << (hFiles + cppFiles + hppFiles) << "\n\n";
        
        // Count lines excluding ThirdParty
        std::string countLines = "find " + basePath + " -name \"*.h\" -o -name \"*.cpp\" -o -name \"*.hpp\" | grep -v ThirdParty | while read file; do wc -l \"$file\" 2>/dev/null; done | awk '{sum += $1} END {print sum}'";
        int totalLines = ExecuteAndGetNumber(countLines);
        
        std::cout << "📏 Total de linhas de código: " << totalLines << "\n\n";
        
        ShowDirectoryBreakdownWithoutThirdParty(basePath);
    }
    
    static void ShowDirectoryBreakdown(const std::string& basePath){
        std::cout << "📂 Distribuição por diretórios principais:\n";
        
        std::string engineCount = "find " + basePath + "Code/Engine -name \"*.h\" -o -name \"*.cpp\" -o -name \"*.hpp\" 2>/dev/null | wc -l";
        std::string editorCount = "find " + basePath + "Code/Editor -name \"*.h\" -o -name \"*.cpp\" -o -name \"*.hpp\" 2>/dev/null | wc -l";
        std::string thirdPartyCount = "find " + basePath + "ThirdParty -name \"*.h\" -o -name \"*.cpp\" -o -name \"*.hpp\" 2>/dev/null | wc -l";
        
        std::cout << "   Engine: " << ExecuteAndGetNumber(engineCount) << " arquivos\n";
        std::cout << "   Editor: " << ExecuteAndGetNumber(editorCount) << " arquivos\n";
        std::cout << "   ThirdParty: " << ExecuteAndGetNumber(thirdPartyCount) << " arquivos\n\n";
    }
    
    static void ShowDirectoryBreakdownWithoutThirdParty(const std::string& basePath){
        std::cout << "📂 Distribuição por diretórios principais:\n";
        
        std::string engineCount = "find " + basePath + "Code/Engine -name \"*.h\" -o -name \"*.cpp\" -o -name \"*.hpp\" 2>/dev/null | wc -l";
        std::string editorCount = "find " + basePath + "Code/Editor -name \"*.h\" -o -name \"*.cpp\" -o -name \"*.hpp\" 2>/dev/null | wc -l";
        
        std::cout << "   Engine: " << ExecuteAndGetNumber(engineCount) << " arquivos\n";
        std::cout << "   Editor: " << ExecuteAndGetNumber(editorCount) << " arquivos\n\n";
        
        std::cout << "💡 Use sem --no-thirdparty para incluir bibliotecas externas\n\n";
    }
    
    static int ExecuteAndGetNumber(const std::string& command){
        FILE* pipe = popen(command.c_str(), "r");
        if (!pipe) return 0;
        
        char buffer[128];
        std::string result = "";
        while (fgets(buffer, sizeof buffer, pipe) != NULL) {
            result += buffer;
        }
        pclose(pipe);
        
        // Convert string to int, removing whitespace
        std::stringstream ss(result);
        int number;
        ss >> number;
        return number;
    }
};