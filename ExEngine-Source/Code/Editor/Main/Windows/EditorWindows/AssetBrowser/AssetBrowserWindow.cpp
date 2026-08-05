#include "AssetBrowserWindow.h"
#include "../EngineConfig/WindowSizeManager.h"
#include "../WindowsUtility/ElementTypeId.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../Utils/FileSystemOpener.h"
#include "../../../../../Engine/File/FileManagement.h"
#include "../../../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../../../../Engine/Core/Runtime/Time/Time.h"
#include "../../../../../Engine/Core/Input/Input.h"
#include "../../../../../Engine/Core/Scene/ECSWorldManager.h"
#include "../../../../../Engine/Core/Utils/Path/PathUtils.h"
#include "../../../../../Engine/Logger/Logger.h"
#include "../../../../../Engine/Core/SpecialFields/SpriteReferenceField/SpriteReference.h"
#include <imgui.h>
#include <SDL.h>
#include <fstream>
#include <sstream>
#include <regex>

AssetBrowserWindow::AssetBrowserWindow(){
    assetManager = AssetManager::GetInstance();
    assetBrowserSelection = std::make_unique<AssetBrowserSelection>();
};

void AssetBrowserWindow::Draw(int phase){
    if(phase != 1) return;

    if(Input::GetButtonDown(SDL_KeyCode::SDLK_SPACE))
    {
        EditorInterfaceGetters::assetBrowserIsOpened = !EditorInterfaceGetters::assetBrowserIsOpened;
        cancelInstantly = true;
    }

    //remove this comment if you want to disable this window when hided
    //if(!cancelInstantly && !EditorInterfaceGetters::assetBrowserIsOpened && currentPosition == targetPosition) return;

    int w = 0, h = 0;
    SDL_GetWindowSize(ExRendererGetters::window, &w, &h);

    // Apply minimum size constraint using WindowSizeManager, but also preserve custom behavior
    ImVec2 minSize = WindowSizeManager::GetMinimumWindowSize();
    ImGui::SetNextWindowSizeConstraints(
        {static_cast<float>(w), std::max(200.0f, minSize.y)}, 
        {static_cast<float>(w), static_cast<float>(h)}
    );

    ImGui::SetNextWindowSize({static_cast<float>(w), ImGui::GetWindowSize().y}, ImGuiCond_Always);

    if(firstUpdate)
    {
        if(EditorInterfaceGetters::assetBrowserIsOpened)
        {
            targetPosition = static_cast<float>(h) + ImGui::GetWindowSize().y;
        }
        else
        {
            targetPosition = static_cast<float>(h) - ImGui::GetWindowSize().y;
        }

        currentPosition = targetPosition;
        firstUpdate = false;
    }
    else
        UpdatePositionTarget(targetPosition, currentPosition, h);

    ImGui::SetNextWindowPos({0, currentPosition});
    
    if(EditorInterfaceGetters::assetBrowserIsOpened || targetPosition != currentPosition)
    {
        ImGui::Begin("Asset Browser##02131231231", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking);
    }
    else
    {
        ImGui::Begin("Asset Browser##02131231231", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
        ImGui::End();
        return;
    }

    auto availableSize = ImGui::GetContentRegionAvail();

    float projectWindowSize = 0;
    ImGui::SetNextWindowSize({static_cast<float>(availableSize.x * 0.3), static_cast<float>(availableSize.y)}, ImGuiCond_Always);
    ImGui::BeginChild("ProjectFolderList", {0, 0}, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
    
    // Left-click on empty space to clear selection
    if (ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && !ImGui::IsAnyItemHovered())
    {
        assetBrowserSelection->Setup("", "", false);
        ElementSelectionController::SetSelected(nullptr);
    }
    
    DrawFolderTree(EditorInterfaceGetters::currentProjectPath);

    // Right-click context menu for empty space
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("ProjectFolderListContextMenu");
    }

    DrawRightClickContextMenu("ProjectFolderListContextMenu");

    projectWindowSize = ImGui::GetWindowSize().x;
    ImGui::EndChild();

    ImGui::SameLine();
    availableSize = ImGui::GetContentRegionAvail();
    
    ImGui::SetNextWindowSize({0, static_cast<float>(availableSize.y)}, ImGuiCond_Always);
    ImGui::BeginChild("ProjectExplorer", {0, 0}, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
    
    // Left-click on empty space to clear selection
    if (ImGui::IsWindowHovered() && (ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && !ImGui::IsAnyItemHovered())
    {
        assetBrowserSelection->Setup("", "", false);
        ElementSelectionController::SetSelected(nullptr);
    }
    
    // Right-click context menu for empty space
    if (ImGui::IsWindowHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        ImGui::OpenPopup("ProjectExplorerContextMenu");
    }

    DrawFolderInspection();

    DrawRightClickContextMenu("ProjectExplorerContextMenu");

    ImGui::EndChild();

    ImGui::End();
};

void AssetBrowserWindow::DrawFolderTree(const std::filesystem::path& path)
{
    for(const auto& subElement : std::filesystem::directory_iterator(path))
    {
        if(std::filesystem::is_directory(subElement))
        {
            auto subElementPath = subElement.path();
            auto treeNodeId = "###" + subElementPath.string();

            if(ImGui::TreeNode((subElementPath.stem().string() + treeNodeId).c_str()))
            {
                if(ImGui::IsItemFocused())
                {
                    if(!IsSelected(treeNodeId, subElementPath))
                    {
                        UpdateSelection(treeNodeId, subElementPath, true);
                    }
                }

                // Right-click context menu for directory tree nodes
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    UpdateSelection(treeNodeId, subElementPath, true);
                    ImGui::OpenPopup("TreeNodeContextMenu");
                }

                DrawRightClickContextMenu("TreeNodeContextMenu");

                DrawFolderTree(subElement);
                ImGui::TreePop();
            }
            else
            {
                // Handle right-click on collapsed tree nodes
                if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
                {
                    UpdateSelection(treeNodeId, subElementPath, true);
                    ImGui::OpenPopup("TreeNodeContextMenu");
                }
                DrawRightClickContextMenu("TreeNodeContextMenu");
            }
        }
        else
        {
            DrawFolderContent(subElement);
        }
    }
};

void AssetBrowserWindow::DrawFolderContent(const std::filesystem::path& entry)
{
    if(IsHidden(entry)) return;

    auto id = "###" + entry.string();
    auto selected = IsSelected(id, entry);
      
    static double lastClickTime = 0.0;

    if(ImGui::Selectable(std::string(entry.filename().string() + id).c_str(), selected))
    {
        double currentTime = ImGui::GetTime();
        double delta = currentTime - lastClickTime;
        lastClickTime = currentTime;

        if (assetBrowserSelection->GetID() == id && delta < 0.30)
        {
            InteractCurrentSelection();
        }

        if(!selected)
        {
            UpdateSelection(id, entry);
        }
    }

    // Right-click context menu for individual items
    if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
    {
        UpdateSelection(id, entry);
        ImGui::OpenPopup("ItemContextMenu");
    }

    DrawRightClickContextMenu("ItemContextMenu");
    StartAssetDragAndDrop(entry);
};

void AssetBrowserWindow::StartAssetDragAndDrop(const std::filesystem::path& entry){
    static nlohmann::json s_currentMovingData;
    if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
        auto elementName = entry.stem().string();
        auto elementType = std::to_string(ElementTypeId::SPRITE);
        s_currentMovingData = SpriteReference(elementName, entry).ToJson();
        ImGui::SetDragDropPayload(elementType.c_str(), &s_currentMovingData, sizeof(nlohmann::json));

        ImGui::Text("Moving %s", elementName.c_str());
        ImGui::EndDragDropSource();
    }
};

void AssetBrowserWindow::UpdatePositionTarget(float& targetPosition, float& currentPosition, int& h)
{
    auto instantly = false;
    if(!cancelInstantly)
        instantly = targetPosition == currentPosition;

    cancelInstantly = false;

    if (EditorInterfaceGetters::assetBrowserIsOpened)
    {
        targetPosition = static_cast<float>(h) - ImGui::GetWindowSize().y;
    }
    else
    {
        targetPosition = static_cast<float>(h) + ImGui::GetWindowSize().y + 50;
    }

    if(instantly)
    {
        currentPosition = targetPosition;
        return;
    }

    if(currentPosition < targetPosition) 
    {
        currentPosition = currentPosition + ImGui::GetIO().DeltaTime * windowVelocity;
        if(currentPosition > targetPosition) currentPosition = targetPosition;
    }
    if(currentPosition > targetPosition) 
    {
        currentPosition = currentPosition - ImGui::GetIO().DeltaTime * windowVelocity;

        if(currentPosition < targetPosition) currentPosition = targetPosition;
    }
};

void AssetBrowserWindow::DrawRightClickContextMenu(const std::string id)
{
    if (ImGui::BeginPopup(id.c_str()))
    {
        if (ImGui::BeginMenu("Create..."))
        {
            if(ImGui::BeginMenu("Hpp Script"))
            {
                if(ImGui::MenuItem("ECSystem"))
                {
                    auto targetFolder = EditorInterfaceGetters::currentProjectPath;
                    auto currentSelectionPath = assetBrowserSelection->GetPath();

                    if(currentSelectionPath != "")
                    {
                        if(currentSelectionPath.has_extension())
                        {
                            currentSelectionPath = currentSelectionPath.parent_path();
                        }

                        if(PathUtils::IsParentPath(targetFolder, currentSelectionPath))
                        {
                            targetFolder = currentSelectionPath;
                        }
                    }
                    
                    CreateHppSystemTemplate(targetFolder / "NewECSystem.hpp");
                }

                if(ImGui::MenuItem("EComponenteS"))
                {
                    auto targetFolder = EditorInterfaceGetters::currentProjectPath;
                    auto currentSelectionPath = assetBrowserSelection->GetPath();

                    if(currentSelectionPath != "")
                    {
                        if(currentSelectionPath.has_extension())
                        {
                            currentSelectionPath = currentSelectionPath.parent_path();
                        }

                        if(PathUtils::IsParentPath(targetFolder, currentSelectionPath))
                        {
                            targetFolder = currentSelectionPath;
                        }
                    }
                    CreateHppComponentTemplate(targetFolder / "NewEComponentS.hpp");
                }
                ImGui::EndMenu();
            }
            
            ImGui::Separator();

            if (ImGui::MenuItem("File"))
            {
                auto targetFolder = EditorInterfaceGetters::currentProjectPath;
                auto currentSelectionPath = assetBrowserSelection->GetPath();

                if(currentSelectionPath != "")
                {
                    if(currentSelectionPath.has_extension())
                    {
                        currentSelectionPath = currentSelectionPath.parent_path();
                    }

                    if(PathUtils::IsParentPath(targetFolder, currentSelectionPath))
                    {
                        targetFolder = currentSelectionPath;
                    }
                }
                
                FileManagement::CreateFile(targetFolder/ "NewFile", "");
                ImGui::CloseCurrentPopup();
            }

            if(ImGui::MenuItem("Directory"))
            {
                auto targetFolder = EditorInterfaceGetters::currentProjectPath;
                auto currentSelectionPath = assetBrowserSelection->GetPath();

                if(currentSelectionPath != "")
                {
                    if(currentSelectionPath.has_extension())
                    {
                        currentSelectionPath = currentSelectionPath.parent_path();
                    }

                    if(PathUtils::IsParentPath(targetFolder, currentSelectionPath))
                    {
                        targetFolder = currentSelectionPath;
                    }
                }

                FileManagement::CreateDirectory(targetFolder / "NewDirectory");
            }
            
            ImGui::EndMenu();
        }

        // Delete option - only show if something is selected
        auto currentSelectionPath = assetBrowserSelection->GetPath();
        if(currentSelectionPath != "" && std::filesystem::exists(currentSelectionPath))
        {
            ImGui::Separator();
            
            if (ImGui::MenuItem("Delete"))
            {
                try
                {
                    if(std::filesystem::is_directory(currentSelectionPath))
                    {
                        std::filesystem::remove_all(currentSelectionPath);
                    }
                    else
                    {
                        std::filesystem::remove(currentSelectionPath);
                    }
                    
                    // Clear selection after deletion
                    assetBrowserSelection->Setup("", "", false);
                    ElementSelectionController::SetSelected(nullptr);
                }
                catch(const std::filesystem::filesystem_error& e)
                {
                    Logger::LogError("Failed to delete: " + std::string(e.what()));
                }
                
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
};

void AssetBrowserWindow::DrawFolderInspection(){
    if(!std::filesystem::exists(assetBrowserSelection->GetPath())) return;
};

void AssetBrowserWindow::UpdateSelection(const std::string& id, const std::filesystem::path& path, bool isDirectory)
{
    assetBrowserSelection->Setup(id, path, isDirectory);
    ElementSelectionController::SetSelected(assetBrowserSelection.get());
};

bool AssetBrowserWindow::IsSelected(const std::string& id, const std::filesystem::path& path) const
{
    auto currentSelection = ElementSelectionController::GetCurrentSelection();
    auto selected = currentSelection != nullptr && currentSelection->GetType() == Asset && dynamic_cast<AssetBrowserSelection*>(currentSelection)->GetID() == id;
    return selected;
};

void AssetBrowserWindow::InteractCurrentSelection() const{
    auto currentSelection = ElementSelectionController::GetCurrentSelection();

    if(currentSelection == nullptr || currentSelection->GetType() != Asset) return;

    auto assetBrowserSelection = dynamic_cast<AssetBrowserSelection*>(currentSelection);

    auto path = assetBrowserSelection->GetPath();

    if(!path.has_extension()) return;

    auto extension = path.extension().string();
    
    if(extension == ".exworld")
    {
        ECSWorldManager::LoadWorld(path);
        EditorInterfaceGetters::worldWithoutPath = false;
    }
    else if(extension == ".h" || extension == ".hpp" || extension == ".cpp")
    {
        FileSystemOpener::OpenFileInSystemEditor(path);
    }
};

// Scans every .hpp already in the project for "fieldName = N" (ComponentId/SystemId) and returns
// the next free value, never below floor. Built-in engine components already occupy 0/1/2
// (Camera/Transform/Sprite), hence the high floor for scripted ComponentIds - this is what removes
// the manual "assign a unique id yourself" step that used to be a silent collision waiting to happen.
unsigned int AssetBrowserWindow::NextAvailableRegistryId(const std::string& fieldName, unsigned int floor) const
{
    auto highest = floor == 0 ? 0u : floor - 1;
    auto projectPath = EditorInterfaceGetters::currentProjectPath;

    if(!std::filesystem::exists(projectPath)) return floor;

    std::regex pattern(fieldName + R"(\s*=\s*(\d+))");

    for(const auto& entry : std::filesystem::recursive_directory_iterator(projectPath))
    {
        if(entry.is_directory() || entry.path().extension() != ".hpp") continue;

        std::ifstream file(entry.path());
        if(!file.is_open()) continue;

        std::stringstream buffer;
        buffer << file.rdbuf();
        std::string source = buffer.str();

        std::smatch match;
        if(std::regex_search(source, match, pattern))
        {
            auto found = static_cast<unsigned int>(std::stoul(match[1].str()));
            if(found > highest) highest = found;
        }
    }

    return highest + 1;
};

void AssetBrowserWindow::CreateHppSystemTemplate(const std::filesystem::path& path) const
{
    auto className = path.stem().string();
    auto systemId = NextAvailableRegistryId("SystemId", 1);

    std::string content =
"#pragma once\n"
"#include \"Code/Engine/Core/ECS/ECSManager.h\"\n"
"#include \"Code/Engine/Core/ECS/InternalRegistry/SystemRegistry.h\"\n"
"\n"
"class " + className + " : public CustomECSystem{\n"
"public:\n"
"    static constexpr unsigned int SystemId = " + std::to_string(systemId) + ";\n"
"\n"
"    " + className + "(){\n"
"        SetSystemName(\"" + className + "\");\n"
"        // AddRequire<YourComponent>(false);\n"
"    };\n"
"\n"
"    void UpdateSystem() override{\n"
"\n"
"    };\n"
"};\n"
"\n"
"REGISTER_SYSTEM(" + className + ", SystemContext::UPDATE)\n";

    FileManagement::CreateFile(path, content);
};

void AssetBrowserWindow::CreateHppComponentTemplate(const std::filesystem::path& path) const
{
    auto className = path.stem().string();
    auto componentId = NextAvailableRegistryId("ComponentId", 1000);

    std::string content =
"#pragma once\n"
"#include \"Code/Engine/Core/ECS/Component/EComponentS.h\"\n"
"#include \"Code/Engine/Core/ECS/InternalRegistry/ComponentRegistry.h\"\n"
"#include \"Code/Engine/Core/Serializer/Demangle.h\"\n"
"\n"
"struct " + className + " : public EComponentS<" + className + ">{\n"
"public:\n"
"    static constexpr unsigned int ComponentId = " + std::to_string(componentId) + ";\n"
"\n"
"    virtual ExSerializedClass Serialize() override{\n"
"        return ExSerializedClass{\n"
"            Demangle(typeid(*this).name()),\n"
"            {}\n"
"        };\n"
"    };\n"
"\n"
"    virtual nlohmann::json ToJson() override { return {}; };\n"
"    virtual void FromJson(const nlohmann::json& json) override {};\n"
"};\n"
"\n"
"REGISTER_COMPONENT(" + className + ")\n";

    FileManagement::CreateFile(path, content);
};

