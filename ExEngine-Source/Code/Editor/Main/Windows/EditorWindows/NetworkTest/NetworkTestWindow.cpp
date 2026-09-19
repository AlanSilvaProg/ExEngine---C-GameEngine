#include "NetworkTestWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Network/Url/NetworkRequest.h"
#include "../../../../../Engine/Network/Socket/SocketConnection.h"
#include "../../../../../Engine/Network/Socket/TransferType.h"
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

NetworkTestWindow::~NetworkTestWindow(){
    if(requestThread.joinable())
        requestThread.join();

    if(socketThread.joinable())
        socketThread.join();
};

void NetworkTestWindow::Draw(const int phase){
    if(phase != 1) return;

    if(!EditorInterfaceGetters::networkTestWindowEnabled) return;

    if(ImGui::Begin("Network Test", &EditorInterfaceGetters::networkTestWindowEnabled))
    {
        DrawNetworkRequestSection();

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Spacing();

        DrawSocketConnectionSection();
    }
    ImGui::End();
};

void NetworkTestWindow::DrawNetworkRequestSection(){
    ImGui::TextUnformatted("NetworkRequest (HTTP)");
    ImGui::TextDisabled("Free public API: jsonplaceholder.typicode.com");

    ImGui::InputText("URL##NetworkRequest", &requestUrl);

    const char* methods[] = { "GET", "POST (sample fields)" };
    ImGui::Combo("Method##NetworkRequest", &requestMethodIndex, methods, IM_ARRAYSIZE(methods));

    ImGui::BeginDisabled(requestInProgress);
    if(ImGui::Button("Send Request"))
        SendTestRequest();
    ImGui::EndDisabled();

    if(requestInProgress)
    {
        ImGui::SameLine();
        ImGui::TextUnformatted("Sending...");
    }

    std::lock_guard<std::mutex> lock(requestResultMutex);
    if(requestHasResult)
    {
        ImGui::TextColored(requestSuccess ? ImVec4(0.4f, 1.0f, 0.4f, 1.0f) : ImVec4(1.0f, 0.4f, 0.4f, 1.0f),
            requestSuccess ? "Success" : "Failed");

        ImGui::InputTextMultiline("##NetworkRequestResult", &requestResultContent, ImVec2(-FLT_MIN, 150.0f), ImGuiInputTextFlags_ReadOnly);
    }
};

void NetworkTestWindow::DrawSocketConnectionSection(){
    ImGui::TextUnformatted("SocketConnection");
    ImGui::TextDisabled("Free public host: example.com (result logged to the Console)");

    ImGui::InputText("Address##SocketConnection", &socketAddress);

    const char* transferTypes[] = { "TCP", "UDP" };
    ImGui::Combo("Transfer Type##SocketConnection", &socketTransferTypeIndex, transferTypes, IM_ARRAYSIZE(transferTypes));

    ImGui::BeginDisabled(socketInProgress);
    if(ImGui::Button("Connect"))
        ConnectTestSocket();
    ImGui::EndDisabled();

    if(socketInProgress)
    {
        ImGui::SameLine();
        ImGui::TextUnformatted("Connecting...");
    }
};

void NetworkTestWindow::SendTestRequest(){
    if(requestInProgress) return;

    if(requestThread.joinable())
        requestThread.join();

    requestInProgress = true;

    const std::string url = requestUrl;
    const bool isPost = requestMethodIndex == 1;

    requestThread = std::thread([this, url, isPost](){
        RequestComposition composition(url);

        if(isPost)
        {
            composition.ChangeUrlMethod(NetworkRequest_OPT_METHOD_POST);
            composition.AddContentTypeAsForm();
            composition.AddPostFields("title", "ExEngine");
            composition.AddPostFields("body", "Hello from NetworkTestWindow");
            composition.AddPostFields("userId", "1");
        }

        composition.Perform();

        std::lock_guard<std::mutex> lock(requestResultMutex);
        requestResultContent = composition.GetContent();
        requestSuccess = composition.Finished() && !requestResultContent.empty();
        requestHasResult = true;

        requestInProgress = false;
    });
};

void NetworkTestWindow::ConnectTestSocket(){
    if(socketInProgress) return;

    if(socketThread.joinable())
        socketThread.join();

    socketInProgress = true;

    const std::string address = socketAddress;
    const TransferType transferType = socketTransferTypeIndex == 0 ? TransferType::TCP : TransferType::UDP;

    socketThread = std::thread([this, address, transferType](){
        SocketConnection connection;
        connection.TryConnectTo(address, transferType);

        socketInProgress = false;
    });
};
