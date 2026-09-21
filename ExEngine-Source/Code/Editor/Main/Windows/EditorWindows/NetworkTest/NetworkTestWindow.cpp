#include "NetworkTestWindow.h"
#include "../../../EditorInterfaceGetters.h"
#include "../../../../../Engine/Network/NetworkManager.h"
#include "../../../../../Engine/Network/BidirectionalCommunication/TransferType.h"
#include "../../../../../Engine/Network/HttpVersion.h"
#include "../../../../../Engine/Network/INetworkObject.h"
#include <imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

NetworkTestWindow::~NetworkTestWindow(){
    if(requestThread.joinable())
        requestThread.join();

    if(socketThread.joinable())
        socketThread.join();

    if(sendThread.joinable())
        sendThread.join();
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

    const char* httpVersions[] = { "Auto", "HTTP/1.1", "HTTP/2", "HTTP/3" };
    ImGui::Combo("HTTP Version##NetworkRequest", &requestHttpVersionIndex, httpVersions, IM_ARRAYSIZE(httpVersions));

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
    ImGui::TextUnformatted("Bidirectional Connection");
    ImGui::TextDisabled("Result logged to the Console");

    const char* transferTypes[] = { "TCP WebSocket", "UDP QUIC" };
    ImGui::Combo("Transfer Type##SocketConnection", &socketTransferTypeIndex, transferTypes, IM_ARRAYSIZE(transferTypes));

    const bool isQuic = socketTransferTypeIndex == 1;
    std::string& currentAddress = isQuic ? quicAddress : socketAddress;
    ImGui::InputText("Address##SocketConnection", &currentAddress);

    ImGui::BeginDisabled(socketInProgress);
    if(ImGui::Button("Connect"))
        ConnectTestSocket();
    ImGui::EndDisabled();

    if(socketInProgress)
    {
        ImGui::SameLine();
        ImGui::TextUnformatted("Connecting...");
    }

    ImGui::BeginDisabled(sendInProgress);
    if(ImGui::Button("Send Message"))
        SendTestMessage();
    ImGui::EndDisabled();

    if(sendInProgress)
    {
        ImGui::SameLine();
        ImGui::TextUnformatted("Sending...");
    }
};

void NetworkTestWindow::SendTestRequest(){
    if(requestInProgress) return;

    if(requestThread.joinable())
        requestThread.join();

    requestInProgress = true;

    const std::string url = requestUrl;
    const bool isPost = requestMethodIndex == 1;

    static const HttpVersion httpVersions[] = { AUTO, HTTP1_1, HTTP2, HTTP3 };
    const HttpVersion httpVersion = httpVersions[requestHttpVersionIndex];

    requestThread = std::thread([this, url, isPost, httpVersion](){
        RequestComposition composition(url);
        composition.ChangeHttpVersion(httpVersion);

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

    const bool isQuic = socketTransferTypeIndex == 1;
    const std::string address = isQuic ? quicAddress : socketAddress;
    const TransferType transferType = isQuic ? TransferType::UDPQUIC : TransferType::TCP;

    socketThread = std::thread([this, address, transferType](){
        NetworkManager::bidirectionalConnectionPtr->TryConnectTo(address, transferType);

        socketInProgress = false;
    });
};

void NetworkTestWindow::SendTestMessage(){
    if(sendInProgress) return;

    if(sendThread.joinable())
        sendThread.join();

    sendInProgress = true;

    sendThread = std::thread([this](){
        NetworkManager::bidirectionalConnectionPtr->SendMessage(INetworkObject{});

        sendInProgress = false;
    });
};
