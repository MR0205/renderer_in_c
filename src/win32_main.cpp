#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <wingdi.h>
#include <strsafe.h>

#include "macro_definitions.h"
#include "platform.h"
#include "win32.h"

internal void 
RestrictCursor(HWND hwnd)
{
    RECT client_rect;
    GetClientRect(hwnd, &client_rect);
    LONG effective_left = client_rect.left + GAME_SCREEN_PADDING_X; 
    LONG effective_top = client_rect.top + GAME_SCREEN_PADDING_Y;
    LONG effective_right = client_rect.right < effective_left + GAME_SCREEN_WIDTH_PX ?
                           client_rect.right : effective_left + GAME_SCREEN_WIDTH_PX;
    LONG effective_bottom = client_rect.bottom < effective_top + GAME_SCREEN_HEIGHT_PX ?
                            client_rect.bottom : effective_top + GAME_SCREEN_HEIGHT_PX;

    POINT effective_left_top = {effective_left, effective_top};
    POINT effective_right_bottom = {effective_right, effective_bottom};
    ClientToScreen(hwnd, &effective_left_top);
    ClientToScreen(hwnd, &effective_right_bottom);
    RECT game_screen_rect_in_screen_corrdinates = {effective_left_top.x, effective_left_top.y,
                                                   effective_right_bottom.x, effective_right_bottom.y};
    ClipCursor(&game_screen_rect_in_screen_corrdinates);
}

internal int32 
GetKeyboardKey(WPARAM vk_code)
{
    int32 ret;
    switch (vk_code)
    {
        case VK_LEFT:
        {
            ret = LEFT_KEY;

        } break;

        case VK_UP:
        {
            ret = UP_KEY;
        } break;

        case VK_RIGHT:
        {
            ret = RIGHT_KEY;
        } break;

        case VK_DOWN:
        {
            ret = DOWN_KEY;
        } break;

        default:
        {
            ASSERT(0)
            // log error
        } break;
    }

    return ret;
}

// TODO: currently only polling the queue on whether the key was pressed or released 
// I dont support examining the combination (in the right order) of key presses
// like ctrl+<number> and shift+<number>, etc. which i definetly need
// neither I store information about whether the button was in repeated pressed state


internal void
Win32CaptureKeyboardInput(MSG * msg, ControlInput * control_input)
{
    bool32 key_down = msg->message == WM_KEYDOWN ? true:false;
    switch(msg->wParam)
    {
        case VK_LEFT:
        case VK_UP:
        case VK_RIGHT:
        case VK_DOWN:
        {
            control_input->keys_pressed[GetKeyboardKey(msg->wParam)] = key_down;
        } break;

        default:
        {
            // char str_output[256];
            // sprintf(str_output, "key down: %llx\n", msg->wParam);
            // OutputDebugStringA(str_output);
            // return DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
}

internal MouseEventType 
GetMouseEventType(UINT message)
{
    MouseEventType ret;
    switch (message)
    {
        case WM_LBUTTONDOWN:
        {
            ret = LEFT_BUTTON_PRESSED;

        } break;

        case WM_LBUTTONUP:
        {
            ret = LEFT_BUTTON_RELEASED;
        } break;

        case WM_RBUTTONDOWN:
        {
            ret = RIGHT_BUTTON_PRESSED;
        } break;

        case WM_RBUTTONUP:
        {
            ret = RIGHT_BUTTON_RELEASED;
        } break;

        default:
        {
            ASSERT(0);
            // log error
        } break;
    }

    return ret;
}

internal void
Win32CaptureMouseMove(MSG * msg, ControlInput * control_input)
{
    int32 x = (int16) msg->lParam;
    int32 y = (int16)((msg->lParam) >> 16);

    // transforming coordinates to game screen coordinates, and don't redirect anything
    // outside of the game screen
    x -= GAME_SCREEN_PADDING_X;
    y -= GAME_SCREEN_PADDING_Y;

    if (0 <= x && x < GAME_SCREEN_WIDTH_PX && 
        0 <= y && y < GAME_SCREEN_HEIGHT_PX)
    {
        control_input->cursor_x = x;
        control_input->cursor_y = y;
    }
}

internal void
Win32CaptureMouseInput(MSG * msg, ControlInput * control_input)
{
    int32 x = (int16) msg->lParam;
    int32 y = (int16)((msg->lParam) >> 16);

    // transforming coordinates to game screen coordinates, and don't redirect anything
    // outside of the game screen
    x -= GAME_SCREEN_PADDING_X;
    y -= GAME_SCREEN_PADDING_Y;

    if (0 <= x && x < GAME_SCREEN_WIDTH_PX && 
        0 <= y && y < GAME_SCREEN_HEIGHT_PX)
    {
        control_input->last_mouse_event.type = GetMouseEventType(msg->message);
        control_input->last_mouse_event.x = x;
        control_input->last_mouse_event.y = y;
    }
}


// NOTE: holding down CTRL key while using mouse wheel will multiply the rotation by 10
internal void
Win32CaptureMouseWheelInput(MSG *msg, ControlInput * control_input)
{
    int32 mouse_wheel_rotation = (int16) (msg->wParam >> 16);
    mouse_wheel_rotation /= WHEEL_DELTA;
    int wheel_boost = ((int16) msg->wParam) & MK_CONTROL? 10:1;
    control_input->mouse_wheel_rotation += mouse_wheel_rotation * wheel_boost; 
}

internal void 
Win32CaptureInput(MSG * msg, ControlInput * control_input)
{
    if (msg->message == WM_KEYUP || msg->message == WM_KEYDOWN)
    {
        Win32CaptureKeyboardInput(msg, control_input);
    } else if (msg->message == WM_LBUTTONDOWN || msg->message == WM_RBUTTONDOWN ||
               msg->message == WM_LBUTTONUP || msg->message == WM_RBUTTONUP)
    {
        Win32CaptureMouseInput(msg, control_input);
    } else if (msg->message == WM_MOUSEMOVE)
    {
        Win32CaptureMouseMove(msg, control_input);
    }
    else if (msg->message == WM_MOUSEWHEEL)
    {
        Win32CaptureMouseWheelInput(msg, control_input);
    }
    // char str_output[256];
    // sprintf(str_output, "key up: %llx\n", msg->wParam);
    // OutputDebugStringA(str_output);
}

internal LRESULT CALLBACK 
Win32WindowProc(HWND hwnd, UINT uMsg,
                WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
        // case WM_SIZE:
        // {
        //     RestrictCursor(hwnd);
        // } break;
        case WM_CLOSE:
        {
            DestroyWindow(hwnd);
        } break;

        case WM_DESTROY:
        {
            PostQuitMessage(0);
        } break;

        default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    return 0;
}

internal void
output_int_to_debug(char * format_string, int32 value)
{
    char temp[256];
    sprintf_s(temp, 256, format_string, value);
    OutputDebugStringA(temp);
}

internal void 
output_real64_to_debug(char * format_string, real64 value)
{
    char temp[256];
    sprintf_s(temp, 256, format_string, value);
    OutputDebugStringA(temp);
}

internal void
DEBUGPrintControlInputArray(ControlInput * control_input)
{
    char ret[256] = "";
    char temp2[256];
    StringCchCatA(ret, 256, "Keyboard keys pressed status (");
    for (int key_index = 0; key_index < KEYBOARD_KEY_SIZE; ++key_index)
    {
        sprintf_s(temp2, 256, "%d, ", control_input->keys_pressed[key_index]);
        StringCchCatA(ret, 256, temp2);
    }


    StringCchCatA(ret, 256, "), ");

    sprintf_s(temp2, 256, "mouse event type, x,y: (%d %d %d), ", 
              control_input->last_mouse_event.type,
              control_input->last_mouse_event.x,
              control_input->last_mouse_event.y);


    StringCchCatA(ret, 256, temp2);
    sprintf_s(temp2, 256, "wheel rotation: (%d)", control_input->mouse_wheel_rotation); 
    StringCchCatA(ret, 256, temp2);
    StringCchCatA(ret, 256, "\n");


    OutputDebugStringA(ret);
}

internal bool32
GetEngineDllWriteFileTime(FILETIME * dll_file_write_time)
{
    HANDLE engine_dll_file_handle = CreateFileA("engine.dll", 0, 0, NULL, OPEN_EXISTING, 
                                                 FILE_ATTRIBUTE_NORMAL, NULL);
    if (engine_dll_file_handle != INVALID_HANDLE_VALUE)
    {
        BY_HANDLE_FILE_INFORMATION file_information;
        BOOL get_file_info_ret = GetFileInformationByHandle(engine_dll_file_handle, &file_information);
        CloseHandle(engine_dll_file_handle);

        if(get_file_info_ret)
        {
             *dll_file_write_time = file_information.ftLastWriteTime;
        } else
        {
            OutputDebugStringA("Couldn't get write time information.\n");
            return false;
        }
    } else
    {
        //OutputDebugStringA("File is not on disk?\n");
        return false;
    }
    return true;
}

internal bool32
LoadEngineDll(EngineDll * engine_dll)
{
    engine_dll->handle = LoadLibraryA("engine.dll");
    if (!engine_dll->handle)
    {
        OutputDebugStringA("Loading dll failed.\n");
        //Log error
        return false;
    } else 
    {
        OutputDebugStringA("Loading dll succeded.\n");
    }
    engine_dll->UpdateStateAndRenderLoadedAddress = (UpdateStateAndRenderPrototype) GetProcAddress(engine_dll->handle, "UpdateStateAndRender");
    if (!engine_dll->UpdateStateAndRenderLoadedAddress)
    {
        OutputDebugStringA("Getting function address failed.\n");
        // Log error
        return false;
    } else
    {
        OutputDebugStringA("Getting function address succeded.\n");
    }

    return true;
}

internal bool32 
WritingDllFinished()
{
    HANDLE still_writing_file_handle = CreateFileA("still_writing", 0, 0, NULL, OPEN_EXISTING, 
                                                 FILE_ATTRIBUTE_NORMAL, NULL);
    bool32 return_val = false;
    if (still_writing_file_handle == INVALID_HANDLE_VALUE)
    {
        return_val = true;
    } else
    {
        return_val = false;
        CloseHandle(still_writing_file_handle);
    }
    return return_val;
}

FileReadResult 
ReadFileIntoMemory(char * file_path)
{
    HANDLE file_handle = CreateFileA(file_path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 
                                     FILE_ATTRIBUTE_NORMAL, NULL);
    //uint8 * return_pointer = NULL;
    FileReadResult result;

    if (file_handle)
    {
        LARGE_INTEGER file_size;
        BOOL get_file_size_ret = GetFileSizeEx(file_handle, &file_size);
        if (get_file_size_ret)
        {
            if (file_size.QuadPart <= 0xFFFFFFFF)
            {
                result.start_pointer = (uint8 *) VirtualAlloc(NULL, file_size.QuadPart, MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);

                if (result.start_pointer)
                {
                    DWORD number_of_bytes_read;
                    BOOL read_file_ret = ReadFile(file_handle, result.start_pointer, (DWORD) file_size.QuadPart, &number_of_bytes_read, NULL);
                    if (read_file_ret)
                    {
                        // TODO: stopped here. why dword bytes read, when i can pass longlong to bytes to read???
                        if (number_of_bytes_read == (DWORD) file_size.QuadPart)
                        {
                            OutputDebugStringA("File read sucess.\n");
                            result.file_size = number_of_bytes_read; //TODO:

                        } else
                        {
                            OutputDebugStringA("File read only partially.\n");
                            // log file read partially
                        }
                    } else
                    {
                        OutputDebugStringA("File read error.\n");
                        // log file read error
                    }
                } else
                {
                    OutputDebugStringA("Failed to allocate memory for a file read with VirtualAlloc.\n");
                }
            } else
            {
                    OutputDebugStringA("Currently supporting only file sizes that fit into uint32.\n");
                // log unsupported file size
            }
        } else
        {
            OutputDebugStringA("Could not get file size.\n");
            // log getting file size failure
        }
        CloseHandle(file_handle);
    } else
    {
        OutputDebugStringA("Could not open the file.\n");
        // log file opening error
    }
    DWORD last_error_code = GetLastError();

    char temp[256];
    sprintf_s(temp, 256, "File read last error code: %d\n", last_error_code);

    OutputDebugStringA(temp);

    ASSERT(result.start_pointer);
    return result;
}

bool32 
FreeFileReadResultFromMemory(FileReadResult file_read_result)
{
    if ( ! VirtualFree(file_read_result.start_pointer, 0, MEM_RELEASE))
    {
        OutputDebugStringA("Error in freeing memory by VirtualFree!");
        return false;
    }
    return true;
}

INT WINAPI 
wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR lpCmdLine, INT nCmdShow)
{
    PlatformProcedures platform_procedures;
    platform_procedures.ReadFileIntoMemory = ReadFileIntoMemory;
    platform_procedures.FreeFileReadResultFromMemory = FreeFileReadResultFromMemory;

    LARGE_INTEGER pc_freq;
    QueryPerformanceFrequency(&pc_freq);

    const wchar_t CLASS_NAME[] = L"My Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = Win32WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_OWNDC;
    wc.hCursor =  LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"My title",
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                               NULL, NULL, hInstance, NULL);
    if (hwnd == NULL) 
    {
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);


    RECT old_cursor_clip_rectangle;
    GetClipCursor(&old_cursor_clip_rectangle);
    RestrictCursor(hwnd);

    SIZE_T requested_game_memory_size = 1 * GIGABYTE; // SIZE_T is defined as __int64
    LPVOID virtual_alloc_return = VirtualAlloc(NULL, requested_game_memory_size,
                                               MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    GameMemory game_memory = {};
    if (virtual_alloc_return) 
    {
        game_memory.permanent_storage = (uint8 *) virtual_alloc_return;
        game_memory.permanent_storage_size = requested_game_memory_size;
    } else
    {
        // log memory error
        return -1;
    }


    FILETIME on_disk_dll_file_write_time;
    EngineDll engine_dll;

    if(!LoadEngineDll(&engine_dll))
    {
        // log error initial loading of the engine dll
        return -1;
    }
    if (!GetEngineDllWriteFileTime(&engine_dll.write_time))
    {
        // log error getting write time of the loaded file, very unlikely?
        return -1;
    }

    MSG msg;
    BOOL message_read;
    BOOL g_Running = 1; // moved out of global scope, as it is not used anywhere else except inside the WinMain
    uint32 target_fps = 60;
    real32 target_ms_per_frame = 1000.0f / (real32) target_fps;

    HDC display_dc = GetDC(hwnd);
    HDC memory_dc = CreateCompatibleDC(display_dc);

    BITMAPINFOHEADER bmiHeader = {};
    bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmiHeader.biWidth = GAME_SCREEN_WIDTH_PX;//client_area_width;
    bmiHeader.biHeight = -GAME_SCREEN_HEIGHT_PX;// -client_area_height;
    bmiHeader.biPlanes = 1;
    bmiHeader.biBitCount = 32;
    bmiHeader.biCompression = BI_RGB;

    BITMAPINFO bmi;
    bmi.bmiHeader = bmiHeader;
    //bmi.bmiColors = NULL;

    void * bits;
    HBITMAP DIBSection = CreateDIBSection(display_dc, &bmi, 
                                          DIB_RGB_COLORS, &bits, NULL, 0);
    if (DIBSection == NULL )
    {
        OutputDebugStringA("Can't allocate DIBSection.\n");
        return -1;
    }
    BitmapOutputBuffer bitmap_output_buffer;
    bitmap_output_buffer.width = GAME_SCREEN_WIDTH_PX;
    bitmap_output_buffer.height = GAME_SCREEN_HEIGHT_PX;
    bitmap_output_buffer.px_in_m = 100.0f;
    bitmap_output_buffer.shift_to_center_of_bitmap_x_px = bitmap_output_buffer.width / 2;
    bitmap_output_buffer.shift_to_center_of_bitmap_y_px = bitmap_output_buffer.height / 2;
    bitmap_output_buffer.bits = (uint32 *) bits;


    ControlInput control_input = {};

    TIMECAPS tc_info;
    timeGetDevCaps(&tc_info, sizeof(tc_info));
    UINT minimum_timer_period_ms = tc_info.wPeriodMin;
    
    if(timeBeginPeriod(minimum_timer_period_ms) == TIMERR_NOCANDO) {
        // todo: Log error
        return -1;
    }

    LARGE_INTEGER prev_pc_val;
    LARGE_INTEGER cur_pc_val;
    LONGLONG ticks_elapsed_cur_frame;
    LONGLONG target_ticks_elapsed_one_frame = pc_freq.QuadPart / target_fps;
    real64 elapsed_cur_frame_ms = 0.0f;
    real64 time_to_sleep_ms;
    uint64 milliseconds_since_start_of_the_game = 0;
    int32 truncated_time_to_sleep_ms;
    QueryPerformanceCounter(&prev_pc_val);
    while (g_Running)
    {
        // reinitialising not persistent Control Input
        control_input.mouse_wheel_rotation = 0;
        // TODO: think through the implementation of left click behaviour
        control_input.last_mouse_event = {}; // typically the mouse click are handled right away, without the need of persisting their state (zeroes the structures makes not handling the same event twice easier, without introduction of flags, or capturing event time)
        while(message_read = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {

            Win32CaptureInput(&msg, &control_input);
            if (msg.message == WM_QUIT) 
            {
                g_Running = 0;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } 

#if 1
        // looking for newer version only once every second
        // due to the increments of not unit length and possibly varying length of time
        // this check allows us to check not exactly unit precision, but based
        // on the length of the last increment, which succeeds if we were placed
        // at the begining of the period for the first time
        // as there is probability that we land on % 1000 == 0 and trigger and additional firing
        // we prevent from that case
        uint64 milliseconds_since_start_of_the_period = milliseconds_since_start_of_the_game % 1000;
        if ( (0 < milliseconds_since_start_of_the_period) && 
             (milliseconds_since_start_of_the_period <= (uint64) elapsed_cur_frame_ms) )
        {
            if (WritingDllFinished() && GetEngineDllWriteFileTime(&on_disk_dll_file_write_time))
            {
                if (CompareFileTime(&engine_dll.write_time, &on_disk_dll_file_write_time) == -1)
                {
                    if(!FreeLibrary(engine_dll.handle))
                    {
                        OutputDebugStringA("Unable to unload the dll.\n");
                        return -1;
                    }
                    if(!LoadEngineDll(&engine_dll))
                    {
                        OutputDebugStringA("Loading of new dll failed for unknown reasons. Terminating.\n");
                        return -1;
                        // failed loading the dll that is on disk and has a write time
                        // strictly older than currently loaded dll

                    } else
                    {
                        engine_dll.write_time = on_disk_dll_file_write_time;
                        // successfully loaded new version of the dll.
                    }
                } else
                {
                    //do nothing, we have actual version
                }
            } else
            {
                // building procedure renamed the old engine.dll but haven't yet finished writing the new one,
                //  waiting for the write complete. we are fine
            }
        }
#endif


        //DEBUGPrintControlInputArray(&control_input);


        (engine_dll.UpdateStateAndRenderLoadedAddress)(&game_memory, &bitmap_output_buffer, 
                                                       &control_input, &platform_procedures,
                                                       elapsed_cur_frame_ms);


        QueryPerformanceCounter(&cur_pc_val);
        ticks_elapsed_cur_frame = cur_pc_val.QuadPart - prev_pc_val.QuadPart;
        ticks_elapsed_cur_frame = ticks_elapsed_cur_frame * 1'000; // preparing to convert to ms
        // now in ms with trancation of fractional part
        elapsed_cur_frame_ms = (real64)ticks_elapsed_cur_frame / (real64)pc_freq.QuadPart; 
        time_to_sleep_ms = target_ms_per_frame - elapsed_cur_frame_ms;
        truncated_time_to_sleep_ms = (int32)time_to_sleep_ms - (int32)minimum_timer_period_ms;
        if (truncated_time_to_sleep_ms > 0)
        {
            Sleep(truncated_time_to_sleep_ms);
        }

        QueryPerformanceCounter(&cur_pc_val);
        ticks_elapsed_cur_frame = cur_pc_val.QuadPart - prev_pc_val.QuadPart;

        while (ticks_elapsed_cur_frame < target_ticks_elapsed_one_frame)
        {
            QueryPerformanceCounter(&cur_pc_val);
            ticks_elapsed_cur_frame = cur_pc_val.QuadPart - prev_pc_val.QuadPart;
        }

        ticks_elapsed_cur_frame = ticks_elapsed_cur_frame * 1'000; 
        elapsed_cur_frame_ms = (real64)ticks_elapsed_cur_frame / (real64)pc_freq.QuadPart; 
        milliseconds_since_start_of_the_game += (uint64) elapsed_cur_frame_ms;
        output_real64_to_debug("%f ms\n", elapsed_cur_frame_ms);
        prev_pc_val = cur_pc_val;


        HGDIOBJ old_bitmap = SelectObject(memory_dc, DIBSection);
        BitBlt(display_dc, GAME_SCREEN_PADDING_X, GAME_SCREEN_PADDING_Y, bitmap_output_buffer.width,
               bitmap_output_buffer.height, memory_dc, 0, 0, SRCCOPY);
        BitBlt(memory_dc, 0, 0, bitmap_output_buffer.width,
               bitmap_output_buffer.height, NULL, 0, 0, BLACKNESS);
        SelectObject(memory_dc, old_bitmap);


        //OutputDebugStringA("All messages extracted.\n");
        /*if (message_read  < 0)
        {
            OutputDebugStringA("GetMessage got an error\n");
        }
        */
    }
    timeEndPeriod(minimum_timer_period_ms);

    ClipCursor(&old_cursor_clip_rectangle);
    return 0;
}

