#pragma once
#include "AssetBrowserSelection.h"
#include "../../EditorWindow.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/AssetManager/AssetManager.h"
#include <filesystem>
#include <memory>
#include <string>

enum class AssetCreationKind{
    File,
    Directory,
    HppSystem,
    HppComponent,
    HppScript
};

class AssetBrowserWindow : public EditorWindow{
private:
    std::shared_ptr<AssetManager> assetManager;
    std::unique_ptr<AssetBrowserSelection> assetBrowserSelection;

    bool cancelInstantly;
    bool firstUpdate = true;
    float targetPosition;
    float currentPosition;
    float windowVelocity = 1000;

    double lastClickTime = 0.0;

    double OpenClickMaxDelta() const;
    double RenameClickMaxDelta() const;

    bool showRenamePopup = false;
    std::filesystem::path renameTargetPath;
    std::string renameExtension;
    char renameBuffer[256] = {};

    bool showCreatePopup = false;
    std::filesystem::path createTargetFolder;
    std::string createExtension;
    AssetCreationKind createKind = AssetCreationKind::File;
    char createBuffer[256] = {};

    bool showNameErrorPopup = false;
    std::string nameErrorMessage;

    void UpdatePositionTarget(float& targetPosition, float& currentPosition, int& h);
    void DrawFolderTree(const std::filesystem::path& path);
    void DrawRightClickContextMenu(const std::string id);
    void DrawFolderContent(const std::filesystem::path& entry);
    void StartAssetDragAndDrop(const std::filesystem::path& entry);
    void DrawFolderInspection();
    void UpdateSelection(const std::string& id, const std::filesystem::path& path, const bool isDirectory = false);
    bool IsSelected(const std::string& id, const std::filesystem::path& path) const;

    std::filesystem::path ResolveCreateTargetFolder() const;
    bool DrawNameInput(const char* label, char* buffer, size_t bufferSize, const std::string& extension);

    void HandleRenameClick(const std::string& id, const std::filesystem::path& path);
    void BeginRename(const std::filesystem::path& path);
    void CommitRename();
    void CancelRename();
    void DrawRenamePopup();

    void BeginCreate(AssetCreationKind kind, const std::filesystem::path& targetFolder, const std::string& extension);
    void CommitCreate();
    void CancelCreate();
    void DrawCreatePopup();

    void DrawNameErrorPopup();

    void InteractCurrentSelection() const;

    void CreateHppSystemTemplate(const std::filesystem::path& path) const;
    void CreateHppComponentTemplate(const std::filesystem::path& path) const;
    void CreateHppScriptTemplate(const std::filesystem::path& path) const;
    unsigned int NextAvailableRegistryId(const std::string& fieldName, unsigned int floor) const;

    inline bool IsHidden(const std::filesystem::path& p)
    {
        bool isHidden = false;

#ifdef _WIN32
        DWORD attrs = GetFileAttributesW(p.wstring().c_str());
        if (attrs != INVALID_FILE_ATTRIBUTES)
            isHidden = (attrs & FILE_ATTRIBUTE_HIDDEN) != 0;
        
        if(isHidden) return true;
#else
        std::string name = p.filename().string();
        isHidden = !name.empty() && name[0] == '.';
#endif

    if(isHidden) return true;

    return !p.has_extension() || p.extension().compare(".exproj") == 0;
    };
public:
    AssetBrowserWindow();
    void Draw(int phase) override; //0 == early 1 == late
};