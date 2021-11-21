#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <wingdi.h>

#define int8 char
#define int16 short
#define int32 int
#define int64 long long

#define uint8 unsigned char
#define uint16 unsigned short
#define uint32 unsigned int
#define uint64 unsigned long long

#define real32 float
#define real64 double

#define bool32 int

#define local_persist static
#define internal static
#define ASSERT(expression) (if(!expression) {*((int *)(0))=0;})

#include "engine.cpp"


// internal void 
// Win32GetClientDimensions(HWND hwnd, DWORD * width, DWORD * height)
// {
//     RECT client_rect;
//     GetClientRect(hwnd, &client_rect);
//     *width  = (DWORD)(client_rect.right - client_rect.left);
//     *height = (DWORD)(client_rect.bottom - client_rect.top);
// }

internal void 
Win32HandleKeyboard(MSG * msg, BitmapOutputBuffer * bitmap_output_buffer)
{
    real32 delta = 0.5;
    bool32 down = true;
    if (msg->message == WM_KEYDOWN)
    {
        down = true;
    }
    else
    {
        //assert(msg->message == WM_KEYUP);
        down = false;
    }

    if (down)
    {
        switch(msg->wParam)
        {
            case 0x57: // 'w'
            {
                g_GlobalState->camera_center_y -= delta;
                OutputDebugStringA("w: down\n");
            } break;

            case 0x41: // 'a'
            {
                g_GlobalState->camera_center_x -= delta;
                OutputDebugStringA("a: down\n");
            } break;

            case 0x53: // 's'
            {
                g_GlobalState->camera_center_y += delta;
                OutputDebugStringA("s: down\n");
            } break;

            case 0x44: // 'd'
            {
                g_GlobalState->camera_center_x += delta;
                OutputDebugStringA("d: down\n");
            } break;

            case VK_PRIOR: // PageUp
            {
                bitmap_output_buffer->px_in_m -= 1;
                OutputDebugStringA("PageUp: down\n");
            } break;

            case VK_NEXT: // PageDown
            {
                bitmap_output_buffer->px_in_m += 1;
                OutputDebugStringA("PageDown: down\n");
            } break;

            default:
            {
                char str_output[256];
                sprintf(str_output, "key press ignored: %#x\n", (UINT)msg->wParam);
                OutputDebugStringA(str_output);
                //return DefWindowProc(hwnd, uMsg, wParam, lParam);
            } break;
        }
    }
    else
    {
        //ignoreing key - ups
    }
}

internal LRESULT CALLBACK 
Win32WindowProc(HWND hwnd, UINT uMsg,
                WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
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

INT WINAPI wWinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance,
                   PWSTR lpCmdLine,
                   INT nCmdShow)
{
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

    // TODO: step-into the code and look at the work of the functions/
    // have not tested anything of this and surprised why the usage
    // of tile map later on from the pointer when it gets evicted from the stack
    // doesnt cause program to crash
    
    uint32 requested_game_memory_size = 200 * 1024 * 1024;
    BYTE * virtual_alloc_rerurn = (BYTE *)VirtualAlloc(NULL, requested_game_memory_size,
                                               MEM_COMMIT|MEM_RESERVE, PAGE_READWRITE);
    if (virtual_alloc_rerurn) 
    {
        g_GameMemory.permanent_storage = virtual_alloc_rerurn;
        g_GameMemory.permanent_storage_size = requested_game_memory_size;
    } else
    {
        // log memory error
        return -1;
    }



    MSG msg;
    BOOL message_read;
    BOOL g_Running = 1; // moved out of global scope, as it is not used anywhere else except inside the WinMain
    uint32 target_fps = 60;
    real32 target_ms_per_frame = 1000.0f / (real32) target_fps;

    HDC display_dc = GetDC(hwnd);
    HDC memory_dc = CreateCompatibleDC(display_dc);

    // DWORD client_area_width;
    // DWORD client_area_height;
    // Win32GetClientDimensions(hwnd, &client_area_width, &client_area_height);

/*
    HBITMAP memory_bitmap = CreateCompatibleBitmap(display_dc, client_area_width, client_area_height);
    BITMAP memory_bitmap_struct;
    GetObject(memory_bitmap, sizeof(BITMAP), &memory_bitmap_struct);
    //HGDIOBJ old_bitmap = SelectObject(memory_dc, memory_bitmap);
    GetObject(memory_bitmap, sizeof(BITMAP), &memory_bitmap_struct);
    Rectangle(memory_dc, 0, 0, 111, 222);
    GetObject(memory_bitmap, sizeof(BITMAP), &memory_bitmap_struct);
    GetObject(memory_dc, sizeof(BITMAP), &memory_bitmap_struct);
*/

    uint32 game_screen_width_px = 1920/2;
    uint32 game_screen_height_px = 1080/2;

    BITMAPINFOHEADER bmiHeader = {};
    bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmiHeader.biWidth = game_screen_width_px;//client_area_width;
    bmiHeader.biHeight = -game_screen_height_px;// -client_area_height;
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
    bitmap_output_buffer.width = game_screen_width_px;
    bitmap_output_buffer.height = game_screen_height_px;
    bitmap_output_buffer.px_in_m = 10.0f;
    bitmap_output_buffer.center_of_bitmap_x_m = 
        0.5 * (real32) bitmap_output_buffer.width / bitmap_output_buffer.px_in_m;
    bitmap_output_buffer.center_of_bitmap_y_m = 
        0.5 * (real32) bitmap_output_buffer.height / bitmap_output_buffer.px_in_m;
    bitmap_output_buffer.bits = (uint32 *) bits;

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
    real64 elapsed_cur_frame_ms;
    real64 time_to_sleep_ms;
    int32 truncated_time_to_sleep_ms;
    QueryPerformanceCounter(&prev_pc_val);

    while (g_Running)
    {
        
        while(message_read = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {

            if (msg.message == WM_KEYUP || msg.message == WM_KEYDOWN) {
                Win32HandleKeyboard(&msg, &bitmap_output_buffer);
            }
            if (msg.message == WM_QUIT) {
                g_Running = 0;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } 

        UpdateStateAndRender(&bitmap_output_buffer);

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
        output_real64_to_debug("%f ms\n", elapsed_cur_frame_ms);
        prev_pc_val = cur_pc_val;


        HGDIOBJ old_bitmap = SelectObject(memory_dc, DIBSection);
        uint32 padding_x = 20;
        uint32 padding_y = 20;
        BitBlt(display_dc, padding_x, padding_y, bitmap_output_buffer.width,
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
    return 0;
}

