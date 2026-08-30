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
#include <cstring>

AssetBrowserWindow::AssetBrowserWindow(){
    assetManager = AssetManager::GetInstance();
    assetBrowserSelection = std::make_unique<AssetBrowserSelection>();
};

void AssetBrowserWindow::Draw(const int phase){
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
    
    auto assetsPath = EditorInterfaceGetters::GetAssetsPath();
    if(!std::filesystem::exists(assetsPath))
    {
        std::filesystem::create_directories(assetsPath);
    }

    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("Assets");
    ImGui::Separator();

    DrawFolderTree(assetsPath);

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

    if(showRenamePopup)
    {
        ImGui::OpenPopup("Rename");
        showRenamePopup = false;
    }
    DrawRenamePopup();

    if(showCreatePopup)
    {
        ImGui::OpenPopup("Create");
        showCreatePopup = false;
    }
    DrawCreatePopup();

    if(showNameErrorPopup)
    {
        ImGui::OpenPopup("Name Error");
        showNameErrorPopup = false;
    }
    DrawNameErrorPopup();

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
                HandleRenameClick(treeNodeId, subElementPath);

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
                HandleRenameClick(treeNodeId, subElementPath);

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

    if(ImGui::Selectable(std::string(entry.filename().string() + id).c_str(), selected))
    {
        double currentTime = ImGui::GetTime();
        double delta = currentTime - lastClickTime;
        lastClickTime = currentTime;

        if(selected && delta < OpenClickMaxDelta())
        {
            InteractCurrentSelection();
        }
        else if(selected && delta <= RenameClickMaxDelta())
        {
            BeginRename(entry);
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
                    BeginCreate(AssetCreationKind::HppSystem, ResolveCreateTargetFolder(), ".hpp");
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::MenuItem("EComponenteS"))
                {
                    BeginCreate(AssetCreationKind::HppComponent, ResolveCreateTargetFolder(), ".hpp");
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::MenuItem("Script"))
                {
                    BeginCreate(AssetCreationKind::HppScript, ResolveCreateTargetFolder(), ".hpp");
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndMenu();
            }

            ImGui::Separator();

            if (ImGui::MenuItem("File"))
            {
                BeginCreate(AssetCreationKind::File, ResolveCreateTargetFolder(), "");
                ImGui::CloseCurrentPopup();
            }

            if(ImGui::MenuItem("Directory"))
            {
                BeginCreate(AssetCreationKind::Directory, ResolveCreateTargetFolder(), "");
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndMenu();
        }

        // Rename / Delete options - only show if something is selected
        auto currentSelectionPath = assetBrowserSelection->GetPath();
        if(currentSelectionPath != "" && std::filesystem::exists(currentSelectionPath))
        {
            ImGui::Separator();

            if(ImGui::MenuItem("Rename"))
            {
                BeginRename(currentSelectionPath);
                ImGui::CloseCurrentPopup();
            }

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

// Blocks '/' and '\\' always, and blocks '.' when UserData points at a non-empty extension -
// callers use this to stop users from typing an extension the system assigns automatically.
static int NameInputTextFilter(ImGuiInputTextCallbackData* data)
{
    if(data->EventChar == '/' || data->EventChar == '\\') return 1;

    auto* extension = static_cast<const std::string*>(data->UserData);
    if(extension != nullptr && !extension->empty() && data->EventChar == '.') return 1;

    return 0;
};

std::filesystem::path AssetBrowserWindow::ResolveCreateTargetFolder() const
{
    auto targetFolder = EditorInterfaceGetters::GetAssetsPath();
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

    return targetFolder;
};

// Draws a name input; when extension isn't empty it's shown as a fixed, non-editable suffix.
bool AssetBrowserWindow::DrawNameInput(const char* label, char* buffer, size_t bufferSize, const std::string& extension)
{
    ImGuiInputTextFlags inputFlags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCharFilter;

    if(extension.empty())
    {
        return ImGui::InputText(label, buffer, bufferSize, inputFlags, NameInputTextFilter, const_cast<std::string*>(&extension));
    }

    float extensionWidth = ImGui::CalcTextSize(extension.c_str()).x;
    float inputWidth = std::max(40.0f, ImGui::GetContentRegionAvail().x - extensionWidth - ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::SetNextItemWidth(inputWidth);

    bool enterPressed = ImGui::InputText(label, buffer, bufferSize, inputFlags, NameInputTextFilter, const_cast<std::string*>(&extension));
    ImGui::SameLine(0, ImGui::GetStyle().ItemInnerSpacing.x);
    ImGui::TextUnformatted(extension.c_str());

    return enterPressed;
};

// The OS/ImGui-configured double-click speed - anything faster is the open/toggle gesture.
double AssetBrowserWindow::OpenClickMaxDelta() const
{
    return ImGui::GetIO().MouseDoubleClickTime;
};

// Rename gets a window right after the double-click speed, so a slow re-click reads as
// intentional rather than a fresh, unrelated click much later.
double AssetBrowserWindow::RenameClickMaxDelta() const
{
    return OpenClickMaxDelta() * 3.0;
};

// Clicking an already-selected item again starts a rename only within the [OpenClickMaxDelta, RenameClickMaxDelta]
// window - faster clicks are treated as the toggle ImGui's TreeNode already performs, slower ones as a fresh click.
void AssetBrowserWindow::HandleRenameClick(const std::string& id, const std::filesystem::path& path)
{
    if(!ImGui::IsItemClicked(ImGuiMouseButton_Left)) return;

    double currentTime = ImGui::GetTime();
    double delta = currentTime - lastClickTime;
    lastClickTime = currentTime;

    if(delta < OpenClickMaxDelta() || delta > RenameClickMaxDelta()) return;

    if(IsSelected(id, path))
    {
        BeginRename(path);
    }
};

void AssetBrowserWindow::BeginRename(const std::filesystem::path& path)
{
    renameTargetPath = path;

    bool isDirectory = std::filesystem::is_directory(path);
    renameExtension = (!isDirectory && path.has_extension()) ? path.extension().string() : "";

    auto name = renameExtension.empty() ? path.filename().string() : path.stem().string();
    strncpy(renameBuffer, name.c_str(), sizeof(renameBuffer) - 1);
    renameBuffer[sizeof(renameBuffer) - 1] = '\0';

    showRenamePopup = true;
};

void AssetBrowserWindow::CommitRename()
{
    auto newStem = std::string(renameBuffer);
    auto originalStem = renameExtension.empty() ? renameTargetPath.filename().string() : renameTargetPath.stem().string();

    if(newStem.empty() || newStem == originalStem)
    {
        CancelRename();
        return;
    }

    auto newPath = renameTargetPath.parent_path() / (newStem + renameExtension);

    if(std::filesystem::exists(newPath))
    {
        nameErrorMessage = "A file or folder named \"" + (newStem + renameExtension) + "\" already exists.";
        showNameErrorPopup = true;
        renameTargetPath.clear();
        ImGui::CloseCurrentPopup();
        return;
    }

    try
    {
        std::filesystem::rename(renameTargetPath, newPath);

        if(renameTargetPath.extension() == ".exworld")
        {
            ECSWorldManager::OnWorldFileRenamed(renameTargetPath, newPath);
        }

        if(assetBrowserSelection->GetPath() == renameTargetPath)
        {
            auto newId = "###" + newPath.string();
            UpdateSelection(newId, newPath, std::filesystem::is_directory(newPath));
        }
    }
    catch(const std::filesystem::filesystem_error& e)
    {
        Logger::LogError("Failed to rename: " + std::string(e.what()));
    }

    renameTargetPath.clear();
    ImGui::CloseCurrentPopup();
};

void AssetBrowserWindow::CancelRename()
{
    renameTargetPath.clear();
    ImGui::CloseCurrentPopup();
};

void AssetBrowserWindow::DrawRenamePopup()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

    if(ImGui::BeginPopup("Rename", flags))
    {
        ImGui::Text("Enter new name:");
        ImGui::Separator();

        if(ImGui::IsWindowAppearing())
        {
            ImGui::SetKeyboardFocusHere();
        }

        bool enterPressed = DrawNameInput("##rename_input", renameBuffer, sizeof(renameBuffer), renameExtension);

        ImGui::Separator();

        if(ImGui::Button("Apply") || enterPressed)
        {
            CommitRename();
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel"))
        {
            CancelRename();
        }

        ImGui::EndPopup();
    }
};

void AssetBrowserWindow::BeginCreate(AssetCreationKind kind, const std::filesystem::path& targetFolder, const std::string& extension)
{
    createKind = kind;
    createTargetFolder = targetFolder;
    createExtension = extension;
    createBuffer[0] = '\0';

    showCreatePopup = true;
};

void AssetBrowserWindow::CommitCreate()
{
    auto name = std::string(createBuffer);

    if(name.empty())
    {
        CancelCreate();
        return;
    }

    auto newPath = createTargetFolder / (name + createExtension);

    if(std::filesystem::exists(newPath))
    {
        nameErrorMessage = "A file or folder named \"" + (name + createExtension) + "\" already exists.";
        showNameErrorPopup = true;
        ImGui::CloseCurrentPopup();
        return;
    }

    switch(createKind)
    {
        case AssetCreationKind::File:
            FileManagement::CreateFile(newPath, "");
            break;
        case AssetCreationKind::Directory:
            FileManagement::CreateDirectory(newPath);
            break;
        case AssetCreationKind::HppSystem:
            CreateHppSystemTemplate(newPath);
            break;
        case AssetCreationKind::HppComponent:
            CreateHppComponentTemplate(newPath);
            break;
        case AssetCreationKind::HppScript:
            CreateHppScriptTemplate(newPath);
            break;
    }

    ImGui::CloseCurrentPopup();
};

void AssetBrowserWindow::CancelCreate()
{
    ImGui::CloseCurrentPopup();
};

void AssetBrowserWindow::DrawCreatePopup()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Appearing);

    ImGuiWindowFlags flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar;

    if(ImGui::BeginPopup("Create", flags))
    {
        ImGui::Text("Enter name:");
        ImGui::Separator();

        if(ImGui::IsWindowAppearing())
        {
            ImGui::SetKeyboardFocusHere();
        }

        bool enterPressed = DrawNameInput("##create_input", createBuffer, sizeof(createBuffer), createExtension);

        ImGui::Separator();

        if(ImGui::Button("Create") || enterPressed)
        {
            CommitCreate();
        }

        ImGui::SameLine();

        if(ImGui::Button("Cancel"))
        {
            CancelCreate();
        }

        ImGui::EndPopup();
    }
};

void AssetBrowserWindow::DrawNameErrorPopup()
{
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(320, 0), ImGuiCond_Appearing);

    if(ImGui::BeginPopup("Name Error", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
    {
        ImGui::TextWrapped("%s", nameErrorMessage.c_str());
        ImGui::Separator();

        if(ImGui::Button("OK"))
        {
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
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
    auto projectPath = EditorInterfaceGetters::GetAssetsPath();

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
"#include \"Code/Engine/ExEngine.h\"\n"
"#include \"Code/Engine/Core/Runtime/App.h\"\n"
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
"    void UpdateSystem(SystemContext systemContext) override{\n"
"        if (!App::isPlaying)\n"
"          return;\n"
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
"#include \"Code/Engine/ExEngine.h\"\n"
"\n"
"struct " + className + " : public EComponentS<" + className + ">{\n"
"public:\n"
"    static constexpr unsigned int ComponentId = " + std::to_string(componentId) + ";\n"
"\n"
"    EX_SERIALIZE_CLASS()\n"
"\n"
"    virtual nlohmann::json ToJson() override { return {}; };\n"
"    virtual void FromJson(const nlohmann::json& json) override {};\n"
"};\n"
"\n"
"REGISTER_COMPONENT(" + className + ")\n";

    FileManagement::CreateFile(path, content);
};

// Plain script: neither a System nor a Component, so it's never picked up by ScriptHotReloadManager's
// DetectScriptKind (which only matches "public EComponentS<"/"public CustomECSystem") - just a regular
// class other scripts can #include, e.g. for shared helpers/enums/data structs.
void AssetBrowserWindow::CreateHppScriptTemplate(const std::filesystem::path& path) const
{
    auto className = path.stem().string();

    std::string content =
"#pragma once\n"
"#include \"Code/Engine/ExEngine.h\"\n"
"\n"
"class " + className + "{\n"
"public:\n"
"\n"
"};\n";

    FileManagement::CreateFile(path, content);
};

