#include "ProjectSelector.h"
#include "../../Engine/Logger/Logger.h"
#include "../../Engine/Core/Utils/Color.h"
#include "../../Engine/File/FileManagement.h"
#include "../../Engine/Core/Engine.h"
#include "../../Engine/Logger/Logger.h"
#include "../Main/EditorInterfaceGetters.h"
#include "../Main/ProjectManager/ProjectInfo.h"
#include "../Main/ProjectManager/ProjectManager.h"
#include "tinyfiledialogs/tinyfiledialogs.h"
#include <string>
#include <filesystem>
#include <imgui.h>
#include <imgui/backends/imgui_impl_sdl2.h>
#include <imgui/backends/imgui_impl_sdlrenderer2.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

void ProjectSelector::CreateProjectSelectorWindow(){
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0){
        Logger::LogError("SDL initialization error with the message: " + std::string(SDL_GetError()));
        return;
    }

    window = SDL_CreateWindow("Ex Engine Project Selector", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 400, SDL_WindowFlags::SDL_WINDOW_RESIZABLE | SDL_WindowFlags::SDL_WINDOW_SHOWN);

    if(!window)
    {
        Logger::LogError("SDL Window creation error with the message: " + std::string(SDL_GetError()));
        return;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);

    if(!renderer)
    {
        Logger::LogError("SDL Renderer creation error with the message: " + std::string(SDL_GetError()));
        return;
    }
    SDL_PumpEvents();
};

void ProjectSelector::CreateProjectSelectorEditor(){
    IMGUI_CHECKVERSION();
    auto editorContext = ImGui::CreateContext();
    ImGui::SetCurrentContext(editorContext);
    ImGui::GetIO().IniFilename = "imgui_editor_project_selector.ini";
    ImGui::GetIO().ConfigFlags = ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDLRenderer2_Init(renderer);

    //Loading Layout
    std::string result;
    if(FileManagement::LoadFile(std::string("project_selector_layout"), result))
        ImGui::LoadIniSettingsFromMemory(result.c_str());
    
    //Loading projects
    std::filesystem::path projectSelectorInfoFilePath = PROJECT_SELECTOR_PATH;
    FileManagement::LoadFromJson(projectSelectorInfoFilePath / "ProjectSelectorInfo", projectSelectorInfo);
};

std::string ProjectSelector::Run(){
    while(true){
        if(CloseByInput())
           break;
        if(!EditorInterfaceGetters::currentProjectPath.empty()) break;
        SDL_SetRenderDrawColor(renderer, Color::BLUE->r, Color::BLUE->g, Color::BLUE->b, Color::BLUE->a);
        SDL_RenderClear(renderer);
        RenderEditor();
        SDL_RenderPresent(renderer);
    }

    return EditorInterfaceGetters::currentProjectPath;
};

void ProjectSelector::RenderEditor(){
    static std::filesystem::path baseProjectsPath = Engine::GetEnginePath() / "Projects";
    static std::filesystem::path basePathToCreate = std::filesystem::path(baseProjectsPath);

    ImGui_ImplSDL2_NewFrame();
    ImGui_ImplSDLRenderer2_NewFrame();
    ImGui::NewFrame();

    //Previous Projects
    auto windowFlags = ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize;
    ImGui::Begin("ProjectSelectorScreen", nullptr, windowFlags);
    int w = 0, h = 0;
    SDL_GetWindowSize(window, &w, &h);
    ImGui::SetWindowSize({static_cast<float>(w), static_cast<float>(h)});
    ImGui::SetWindowPos({0,0});

    ImGui::BeginGroup();
    ImGui::Begin("Projects", nullptr, windowFlags);
    ImGui::SetWindowSize({static_cast<float>(w * 0.75), static_cast<float>(h)});
    ImGui::SetWindowPos({0,0});

    if(projectSelectorInfo.projectInfo.size() == 0)
    {
        float windowWidth = ImGui::GetWindowSize().x;
        float textWidth = ImGui::CalcTextSize("Nenhum projeto foi criado ainda").x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text("Nenhum projeto foi criado ainda");
    }
    else
    {
        auto index = 0;
        ProjectInfo* projectInfoToRemove = nullptr;
        for(auto& project : projectSelectorInfo.projectInfo)
        {
            auto windowSize = ImGui::GetContentRegionAvail();
            ImGui::BeginChild(std::string("Project_" + project.projectName + project.projectPath + std::to_string(index)).c_str(), {windowSize.x, 120}, ImGuiChildFlags_Borders, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar);

            ImGui::TextWrapped("%s", ("Project Name: " + project.projectName).c_str());
            ImGui::TextWrapped("%s", ("Project Path: " + project.projectPath).c_str());

            float availableWidth = ImGui::GetContentRegionAvail().x;
            float buttonWidth = std::min(100.0f, availableWidth * 0.8f);
            float offsetX = (availableWidth - buttonWidth) * 0.5f;

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

            if (ImGui::Button("Open", ImVec2(buttonWidth, 0))) {
                EditorInterfaceGetters::currentProjectPath = project.GetFullPath();
            }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offsetX);

            if (ImGui::Button("Remove", ImVec2(buttonWidth, 0))) {
                projectInfoToRemove = &project;
            }

            ImGui::EndChild();
            index++;
        }

        if(projectInfoToRemove != nullptr)
        {
            projectSelectorInfo.projectInfo.erase(std::remove_if(projectSelectorInfo.projectInfo.begin(), projectSelectorInfo.projectInfo.end(), [projectInfoToRemove](ProjectInfo& proj){
                return &proj == projectInfoToRemove;
            }), projectSelectorInfo.projectInfo.end());
            projectInfoToRemove = nullptr;
        }
    }

    ImGui::End();
    ImGui::EndGroup();

    ImGui::SameLine();
    ImGui::BeginGroup();

    ImGui::Begin("Project Tool Kit", nullptr, windowFlags);
    ImGui::SetWindowSize({static_cast<float>(w * 0.25), static_cast<float>(h)});
    ImGui::SetWindowPos({static_cast<float>(w * 0.75), 0});

    auto static creating = false;
    auto static openProjectFailed = false;

    if(!creating)
    {
        if(ImGui::Button("Open From PC")){
            openProjectFailed = false;
            const char* folder = tinyfd_selectFolderDialog(
                "Select project folder",
                baseProjectsPath.c_str()
            );

            if(folder != nullptr)
            {
                if(ProjectManager::IsValidProject(folder))
                {
                    projectSelectorInfo.projectInfo.push_back(ProjectInfo(ProjectManager::GetProjectName(folder), (std::filesystem::path(folder).parent_path().parent_path())));
                    SaveProjectInfo();
                }
                else
                {
                    openProjectFailed = true;
                    Logger::LogError("Project path selected is invalid, .exproject file is needed");
                }
            }
        }

        if(openProjectFailed)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::TextWrapped("Failed to open the project, check the logs for more information");
            ImGui::PopStyleColor();
        }

        if(ImGui::Button("Create new")){
            openProjectFailed = false;
            creating = true;
        }
    }
    else 
    {
        if(ImGui::SmallButton("<###create-new-project-back-button"))
        {
            creating = false;
        }

        static bool emptyNameDetected;
        static std::string projectName;

        ImGui::Text("Project Name");

        if(ImGui::InputText("###ProjectName", &projectName))
            emptyNameDetected = false;

        if(emptyNameDetected)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 0, 0, 1));
            ImGui::TextWrapped("name is needed to create a new project");
            ImGui::PopStyleColor();
        }
        
        ImGui::Text("Folder output");

        auto outputTarget = std::string(basePathToCreate / projectName);
        ImGui::InputText("###OutputFullPath", &outputTarget);

        ImGui::SameLine();
        if(ImGui::SmallButton("Change"))
        {
            const char* folder = tinyfd_selectFolderDialog(
                "Select project folder",
                basePathToCreate.c_str()
            );
            if(folder != nullptr)
                basePathToCreate = folder;
        }

        if(ImGui::Button("Create Project")){
            if(projectName.empty())
            {
                emptyNameDetected = true;
            }
            else
            {
                if(ProjectInfo projectInfo; ProjectManager::CreateBaseProjectAt(basePathToCreate, projectName, projectInfo))
                {
                    projectSelectorInfo.projectInfo.push_back(projectInfo);
                    SaveProjectInfo();
                    creating = false;
                }
                else
                {
                    Logger::LogError("Project creation failed");
                }
            }
        }
    }

    ImGui::End();
    ImGui::EndGroup();

    ImGui::End();
    //ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
};

void ProjectSelector::SaveProjectInfo(){
    std::filesystem::path projectSelectorInfoFilePath = PROJECT_SELECTOR_PATH;
    FileManagement::SaveFile(projectSelectorInfoFilePath / "ProjectSelectorInfo", projectSelectorInfo.ToJson().dump());
};

//LifeCycle
bool ProjectSelector::CloseByInput() const
{
    SDL_Event sdlEvent;
    while(SDL_PollEvent(&sdlEvent))
    {
        ImGui_ImplSDL2_ProcessEvent(&sdlEvent);
        if(sdlEvent.type == SDL_QUIT || sdlEvent.key.keysym.sym == SDL_KeyCode::SDLK_ESCAPE)
            return true;
    }
    return false;
};

ProjectSelector::~ProjectSelector(){
    SDL_Quit();
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    //layout persistence
    size_t size;
    const char* iniData = ImGui::SaveIniSettingsToMemory(&size);
    FileManagement::SaveFile(std::string("project_selector_layout"), iniData);

    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
};