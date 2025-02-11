#include "App.h"

#ifdef GAME_BUILD
bool App::isPlaying = true;
#else
bool App::isPlaying = false;
#endif
bool App::isDebugMode = false;
bool App::isEditorMode = false;