#pragma once
#include "../../JsonUtility/JsonUtility.h"
#include "../../Logger/Logger.h"
#include <nlohmann/json.hpp>
#include <curl/curl.h>
#include <string>
#include <vector>
#include <unordered_map>

class NetworkRequest;

#ifndef NetworkRequest_OPT_METHOD_GET
#define NetworkRequest_OPT_METHOD_GET "GET"
#define NetworkRequest_OPT_METHOD_POST "POST"
#define NetworkRequest_OPT_METHOD_PUT "PUT"
#define NetworkRequest_OPT_METHOD_PATCH "PATCH"
#define NetworkRequest_OPT_METHOD_DELETE "DELETE"
#define NetworkRequest_OPT_METHOD_HEAD "HEAD"
#define NetworkRequest_OPT_METHOD_OPTIONS "OPTIONS"
#endif

#ifndef FORMAT_POST_FIELD(variable, value)
#define FORMAT_POST_FIELD(variable, value) variable + "=" + value
#endif

#ifndef BEARER_TOKEN_HEADER
#define BEARER_TOKEN_HEADER "Authorization: Bearer "
#endif

//CONTENT TYPE FOR HTTPHEADER OPT
#ifndef CONTENT_TYPE_HEADER
#define CONTENT_TYPE_HEADER "Content-Type: "
#endif

//example: R"({"username":"alan", "level":10})" << Post field
#ifndef CONTENT_TYPE_JSON
#define CONTENT_TYPE_JSON "Content-Type: application/json"
#endif

//example: username=Alan&level=10 << Post field
#ifndef CONTENT_TYPE_FORM
#define CONTENT_TYPE_FORM "Content-Type: application/x-www-form-urlencoded"
#endif

//example: std::vector<unsigned char> data   send through postfield data.data() and postfieldsize data.size()
#ifndef CONTENT_TYPE_BYTE
#define CONTENT_TYPE_BYTE "Content-Type: application/octet-stream"
#endif

struct RequestComposition{
private:
    std::string url;
    std::string urlMethod = "GET";
    std::vector<std::string> postFields;
    std::vector<std::string> headerList;
    std::string content;
    bool hasFinished;
    bool success;

    friend class NetworkRequest;
public:
    RequestComposition() = default;
    RequestComposition(std::string url) : url(url){};
    RequestComposition(std::string url, std::string urlMethod) : url(url), urlMethod(urlMethod) {};
    RequestComposition(std::string url, std::string urlMethod, std::vector<std::string> postFields) : url(url), urlMethod(urlMethod), postFields(postFields) {};
    RequestComposition(std::string url, std::string urlMethod, std::vector<std::string> postFields, std::vector<std::string> headerList) : url(url), urlMethod(urlMethod), postFields(postFields), headerList(headerList) {};
    ~RequestComposition() = default; // ToDo if awaitable, clean every allocated memory tasks resource

    RequestComposition* Perform();

    // Personalization
    inline RequestComposition* ChangeUrl(std::string url) { this->url = url; return this; };
    /**
    * Change the http method.
    *
    * @param method Can be any NetworkRequest_OPT_METHOD Macro.
    */
    inline RequestComposition* ChangeUrlMethod(const std::string method){ urlMethod = method; return this; };
    inline RequestComposition* AddPostFields(const std::string key, const std::string value){ postFields.push_back(FORMAT_POST_FIELD(key, value)); return this; };

    inline RequestComposition* AddCustomHeader(const std::string headerValue) { headerList.push_back(headerValue); return this; };
    inline RequestComposition* AddCustomHeader(const std::string header, const std::string value) { AddCustomHeader(header+value); return this; };

    inline RequestComposition* AddBearerToken(const std::string token) { AddCustomHeader(BEARER_TOKEN_HEADER+token); return this; };
    /** 
    * @param contentType Can be application/octet-stream or application/x-www-form-urlencoded or application/json
    */
    inline RequestComposition* AddContentType(const std::string contentType) { AddCustomHeader(CONTENT_TYPE_HEADER+contentType); };

    inline RequestComposition* AddContentTypeAsJson() { AddCustomHeader(CONTENT_TYPE_JSON); };
    inline RequestComposition* AddContentTypeAsForm() { AddCustomHeader(CONTENT_TYPE_FORM); };
    inline RequestComposition* AddContentTypeAsBytes() { AddCustomHeader(CONTENT_TYPE_BYTE); };

    //ToDo make it awaitable
    inline const bool Finished() const { return hasFinished; };

    // Getter's 
    inline std::string GetContent() { return content; };
    inline std::vector<std::string>& GetPostFields() { return postFields; };
    inline nlohmann::json GetContentAsJson() { return JsonUtility::GetJsonFromString(content); };
};

class NetworkRequest{
private:
    inline static size_t WriteCallback(char* contents, size_t size, size_t nmemb, void* userp){
        size_t totalSize = size * nmemb;
        static_cast<std::string*>(userp)->append(contents, totalSize);
        return totalSize;
    };
public:
    inline static void DoRequest(RequestComposition* composition){
        auto curl = curl_easy_init();

        curl_easy_setopt(curl, CURLOPT_URL, composition->url.c_str());
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, composition->urlMethod.c_str());

        curl_slist* headerList = nullptr;
        if(!composition->headerList.empty()){
            for(auto header : composition->headerList)
            {
                headerList = curl_slist_append(headerList, header.c_str());
            }
            curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerList);
        }

        std::string postFields;
        if(!composition->postFields.empty()){
            for(const auto field : composition->postFields){
                if(!postFields.empty())
                    postFields+= "&";
                postFields += field;
            }
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, postFields.c_str());
        }

        std::string response;
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        CURLcode result = curl_easy_perform(curl);
        composition->success = true;

        if (result != CURLE_OK)
        {
            Logger::Log("Network Request failed: " + std::string(curl_easy_strerror(result)));
            composition->success = false;
        }

        composition->content = response;
        composition->hasFinished = true;

        curl_slist_free_all(headerList);
        curl_easy_cleanup(curl);
    };
};

inline RequestComposition* RequestComposition::Perform(){ NetworkRequest::DoRequest(this); return this; };