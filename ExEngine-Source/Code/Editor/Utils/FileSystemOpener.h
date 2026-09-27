#pragma once
#include <filesystem>

// Opens a file in the editor set via RuntimeSettings::SetExternalTextEditorPath (Project
// Settings), or the OS default otherwise (Windows: ShellExecuteW; macOS/Linux: open/xdg-open).
class FileSystemOpener {
public:
    static bool OpenFileInSystemEditor(const std::filesystem::path& filePath);

private:
    FileSystemOpener() = default;
};