Como usar o FileWatcher System
O sistema FileWatcher já está automaticamente integrado no EditorInterface e funcionando. Aqui está como usar:

1. Uso Automático 
O FileWatcher já está ativo quando você inicia o editor e:

Monitora automaticamente o diretório do projeto atual
Observa pastas Assets/ e Scripts/ se existirem
Filtra tipos de arquivo relevantes (imagens, scripts, etc.)
Registra eventos no log da engine
Salva/carrega configuração automaticamente
2. Uso Programático Manual
Se quiser criar uma instância personalizada:

#include "FileWatcher/FileWatcher.h"

// Criar instância
auto fileWatcher = std::make_unique<FileWatcher>();

// Configurar caminhos para monitorar
fileWatcher->AddWatchPath("/caminho/para/projeto", true);
fileWatcher->SetFileFilter({".cpp", ".h", ".lua", ".png"});

// Registrar callbacks para eventos
fileWatcher->OnFileCreated += [](const FileEvent& event) {
    Logger::Log("Arquivo criado: " + event.filePath);
};

fileWatcher->OnFileChanged += [](const FileEvent& event) {
    Logger::Log("Arquivo modificado: " + event.filePath);
};

fileWatcher->OnFileDeleted += [](const FileEvent& event) {
    Logger::Log("Arquivo deletado: " + event.filePath);
};

// Iniciar monitoramento
fileWatcher->Start();
3. Configuração Avançada

// Filtros de exclusão (regex)
fileWatcher->SetExcludePat