#pragma once
#include "../ElementSelectionController.h"
#include <string>
#include <filesystem>

class AssetBrowserSelection : public IEditorSelectable{
private:
    std::filesystem::path assetPath;
    std::string assetId;
    bool isDirectory;
public:
    inline virtual EditorSelectableType GetType() override { return EditorSelectableType::Asset; };

    inline std::filesystem::path GetPath() const { return assetPath; };
    inline std::string GetID() const { return assetId; };

    inline void Setup(const std::string& id, const std::filesystem::path& path, const bool directory)
    {
        assetPath = path;
        assetId = id;
        isDirectory = directory;
    };
};