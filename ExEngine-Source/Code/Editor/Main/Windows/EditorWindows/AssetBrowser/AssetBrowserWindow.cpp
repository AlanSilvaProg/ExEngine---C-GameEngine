#include "AssetBrowserWindow.h"
#include "../../../../../Engine/Core/Rendering/Renderer/ExRendererGetters.h"
#include "../../../../../Engine/Core/Runtime/Time/Time.h"
#include "../../../../../Engine/Core/Input/Input.h"
#include "../../../EditorInterfaceGetters.h"
#include <imgui.h>
#include <SDL.h>

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

    ImGui::SetNextWindowSizeConstraints({static_cast<float>(w), 200}, {static_cast<float>(w), static_cast<float>(h)});

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
    
    DrawFolderTree(EditorInterfaceGetters::currentProjectPath);

    projectWindowSize = ImGui::GetWindowSize().x;
    ImGui::EndChild();

    ImGui::SameLine();
    availableSize = ImGui::GetContentRegionAvail();
    
    ImGui::SetNextWindowSize({0, static_cast<float>(availableSize.y)}, ImGuiCond_Always);
    ImGui::BeginChild("ProjectExplorer", {0, 0}, ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoTitleBar);
    
    DrawFolderInspection();

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

                DrawFolderTree(subElement);
                ImGui::TreePop();
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
      
    if(ImGui::Selectable(std::string(entry.stem().string() + id).c_str(), selected))
    {
        if(!selected)
        {
            UpdateSelection(id, entry);
        }
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

void AssetBrowserWindow::DrawFolderInspection(){
    if(!std::filesystem::exists(assetBrowserSelection->GetPath())) return;
};

void AssetBrowserWindow::UpdateSelection(const std::string& id, const std::filesystem::path& path, bool isDirectory)
{
    assetBrowserSelection->Setup(id, path, isDirectory);
    ElementSelectionController::SetSelected(assetBrowserSelection.get());
}

bool AssetBrowserWindow::IsSelected(const std::string& id, const std::filesystem::path& path) const
{
    auto currentSelection = ElementSelectionController::GetCurrentSelection();
    auto selected = currentSelection != nullptr && currentSelection->GetType() == Asset && dynamic_cast<AssetBrowserSelection*>(currentSelection)->GetID() == id;
    return selected;
}