#if !defined(WIN32_H)

#include "platform.h"

#define GAME_SCREEN_WIDTH_PX 1920/2
#define GAME_SCREEN_HEIGHT_PX 1080/2

#define GAME_SCREEN_PADDING_X 20
#define GAME_SCREEN_PADDING_Y 20

struct EngineDll
{
    HMODULE handle;
    UpdateStateAndRenderPrototype UpdateStateAndRenderLoadedAddress;
    FILETIME write_time;
};

FileReadResult 
ReadFileIntoMemory(char * file_path);

bool32 
FreeFileReadResultFromMemory(FileReadResult file_read_result);

INT WINAPI 
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, INT nCmdShow);

#define WIN32_H
#endif
