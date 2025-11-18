#include "EditorInterfaceGetters.h"

std::shared_ptr<Engine> EditorInterfaceGetters::engine = nullptr;
bool EditorInterfaceGetters::sceneViewEnabled = false;
bool EditorInterfaceGetters::gameViewEnabled = false;
bool EditorInterfaceGetters::projectSettingsEnabled = false;
bool EditorInterfaceGetters::assetBrowserIsOpened = false;
std::filesystem::path EditorInterfaceGetters::currentProjectPath;

std::map<std::string, std::unique_ptr<SpriteInformation>> EditorInterfaceGetters::defaultIconsInformation;