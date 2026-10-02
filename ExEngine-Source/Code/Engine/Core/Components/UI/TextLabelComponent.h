#pragma once
#include "../../ECS/ECSManager.h"
#include "../../ECS/InternalRegistry/ComponentRegistry.h"
#include "../../AssetManager/AssetManager.h"
#include "../../SpecialFields/FontReferenceField/FontReference.h"
#include "../../Rendering/Layer/LayerAttributes.h"
#include "../../Rendering/Renderer/ExRendererGetters.h"
#include "../../Utils/ExRect.h"
#include <SDL2/SDL_ttf.h>
#include <glm/glm.hpp>
#include <string>
#include <filesystem>
#include <memory>

struct TextLabelComponent : public EComponentS<TextLabelComponent>{
private:
    std::shared_ptr<AssetManager> assetManager;
    std::string loadedFontId;

    // The texture is only ever (re)created at the fixed MAX_FONT_SIZE (see FontAssetReference),
    // so this caches its natural pixel size - used both as the SDL_RenderCopyEx source rect and
    // as the baseline the system scales down to the requested display `size`.
    inline void GetTextureInformation(){
        if(textureRect != nullptr){
            delete textureRect;
            textureRect = nullptr;
        }

        if(texture == nullptr) return;

        SDL_Point point;
        SDL_QueryTexture(texture, NULL, NULL, &point.x, &point.y);
        textureRect = new SDL_Rect { 0, 0, point.x, point.y };
    };

public:
    static constexpr unsigned int ComponentId = 7;
    static constexpr const char* ComponentGroup = "UI";

    FontReference fontReference;
    TTF_Font* font;
    std::string textContent;
    int size = 16;
    LayerAttributes layerAttributes;
    // Default box big enough to hold a line or two of text at the default `size`, so a freshly
    // added component has a visible, usable gizmo instead of a zero-area box.
    ExRect exRect = ExRect(glm::vec2(0.0f, 0.0f), glm::vec2(200.0f, 50.0f));
    bool showGizmo = true;
    bool fit = false;

    SDL_Texture* texture;
    SDL_Rect* textureRect;

    TextLabelComponent(){
        font = nullptr;
        texture = nullptr;
        textureRect = nullptr;
        assetManager = AssetManager::GetInstance();
    };

    TextLabelComponent(const TextLabelComponent& textLabel){
        fontReference = textLabel.fontReference;
        textContent = textLabel.textContent;
        size = textLabel.size;
        layerAttributes = textLabel.layerAttributes;
        exRect = textLabel.exRect;
        showGizmo = textLabel.showGizmo;
        fit = textLabel.fit;
        font = nullptr;
        texture = nullptr;
        textureRect = nullptr;
        assetManager = AssetManager::GetInstance();

        SetFont(fontReference.id, fontReference.path);
    };

    ~TextLabelComponent(){
        if(texture != nullptr) SDL_DestroyTexture(texture);
        if(textureRect != nullptr) delete textureRect;
        assetManager->FreeAsset(loadedFontId);
    };

    inline TextLabelComponent& SetFont(std::string fontId, std::filesystem::path fontPath){
        // An empty id (e.g. the Inspector's "Clear" button) has no font of its own to fall back
        // to - using the engine's default font here keeps `font` always valid, instead of leaving
        // it pointed at whatever FreeAsset below may have just closed.
        const std::string resolvedFontId = fontId.empty() ? AssetManager::DEFAULT_FONT_ID : fontId;

        if(!loadedFontId.empty() && loadedFontId != resolvedFontId)
            assetManager->FreeAsset(loadedFontId);

        fontReference.id = fontId;
        fontReference.path = fontPath;

        font = fontId.empty()
            ? assetManager->GetFont(AssetManager::DEFAULT_FONT_ID, "")
            : assetManager->GetFont(fontReference.id, fontReference.path.string());

        loadedFontId = resolvedFontId;

        TextUpdated();

        return *this;
    };

    inline void TextUpdated(){
        if(texture != nullptr) {
            SDL_DestroyTexture(texture);
            texture = nullptr;
        }

        if(font == nullptr || textContent.empty()) return;

        //ToDo change color
        auto surface = TTF_RenderText_Blended(font, textContent.c_str(), {255, 255, 255, 255});
        if(surface == nullptr) return;

        texture = SDL_CreateTextureFromSurface(ExRendererGetters::renderer, surface);
        SDL_FreeSurface(surface);

        GetTextureInformation();
    };

    EX_SERIALIZE_CLASS(
        EX_SERIALIZER_CB((*this), fontReference, true, ([this](){ SetFont(fontReference.id, fontReference.path); })),
        EX_SERIALIZER_CB((*this), textContent, true, ([this](){ TextUpdated(); })),
        EX_SERIALIZER((*this), size, true),
        EX_SERIALIZER((*this), layerAttributes, true),
        EX_SERIALIZER((*this), exRect, true),
        EX_SERIALIZER((*this), showGizmo, true),
        EX_SERIALIZER((*this), fit, true)
    )

    virtual nlohmann::json ToJson() override {
        return {
            {"fontReference", fontReference.ToJson()},
            {"textContent", textContent},
            {"size", size},
            {"layerAttributes", layerAttributes.ToJson()},
            {"exRect", exRect.ToJson()},
            {"showGizmo", showGizmo},
            {"fit", fit}
        };
    }

    virtual void FromJson(const nlohmann::json& json) override {
        if (json.contains("fontReference"))
            fontReference.FromJson(json["fontReference"]);

        if (json.contains("textContent"))
            textContent = json["textContent"];

        if (json.contains("size"))
            size = json["size"].get<int>();

        if (json.contains("layerAttributes"))
            layerAttributes.FromJson(json["layerAttributes"]);

        if (json.contains("exRect"))
            exRect.FromJson(json["exRect"]);
        if (json.contains("showGizmo"))
            showGizmo = json["showGizmo"].get<bool>();
        if (json.contains("fit"))
            fit = json["fit"].get<bool>();

        SetFont(fontReference.id, fontReference.path);
    }
};

REGISTER_COMPONENT(TextLabelComponent)
