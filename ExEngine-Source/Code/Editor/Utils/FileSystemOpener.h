#pragma once
#include <filesystem>

/**
 * Utility class for opening files in the system's default editor/application.
 * 
 * This class provides cross-platform support for opening files:
 * - Windows: Uses ShellExecuteW API for Unicode support
 * - macOS: Uses system() call with "open" command
 * - Linux: Uses system() call with "xdg-open" command
 */
class FileSystemOpener {
public:
    /**
     * Opens a file in the system's default editor/application.
     * 
     * The function validates the file path, converts relative paths to absolute,
     * checks file existence, and provides comprehensive error logging.
     * 
     * @param filePath The path to the file to open (can be relative or absolute)
     * @return true if the file was successfully opened, false otherwise
     */
    static bool OpenFileInSystemEditor(const std::filesystem::path& filePath);

private:
    // Private constructor to prevent instantiation (utility class)
    FileSystemOpener() = default;
};