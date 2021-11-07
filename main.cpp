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

#define bool32 int

#define ASSERT(expression) (if(!expression) {*((int *)(0))=0;})

BOOL g_Running;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
                            WPARAM wParam, LPARAM lParam);

int mouse_x;
int mouse_y;


void get_client_dimensions(HWND hwnd, unsigned int * width, unsigned int * height)
{
    RECT client_rect;
    GetClientRect(hwnd, &client_rect);
    *width = client_rect.right - client_rect.left;
    *height = client_rect.bottom - client_rect.top;
}

struct global_state
{
    uint32 offset_x;
    uint32 offset_y;

} global_state;

void HandleKeyboard(MSG * msg)
{
    uint32 delta = 5;
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
                global_state.offset_y += delta;
                OutputDebugStringA("w: down\n");
            } break;

            case 0x41: // 'a'
            {
                global_state.offset_x -= delta;
                OutputDebugStringA("a: down\n");
            } break;

            case 0x53: // 's'
            {
                global_state.offset_y -= delta;
                OutputDebugStringA("s: down\n");
            } break;

            case 0x44: // 'd'
            {
                global_state.offset_x += delta;
                OutputDebugStringA("d: down\n");
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

INT WINAPI wWinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance,
                   PWSTR lpCmdLine,
                   INT nCmdShow)
{
    LARGE_INTEGER pc_freq;
    QueryPerformanceFrequency(&pc_freq);

    const wchar_t CLASS_NAME[] = L"My Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.style = CS_OWNDC;
    wc.hCursor =  LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0, CLASS_NAME, L"this is title of the window",
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                               NULL, NULL, hInstance, NULL);
    if (hwnd == NULL) 
    {
        return 0;
    }
    ShowWindow(hwnd, nCmdShow);


    MSG msg;
    BOOL message_read;
    g_Running = 1;

    HDC display_dc = GetDC(hwnd);
    HDC memory_dc = CreateCompatibleDC(display_dc);

    unsigned int window_width;
    unsigned int window_height;
    get_client_dimensions(hwnd, &window_width, &window_height);

    HBITMAP memory_bitmap = CreateCompatibleBitmap(display_dc, window_width, window_height);
    BITMAP memory_bitmap_struct;
    GetObject(memory_bitmap, sizeof(BITMAP), &memory_bitmap_struct);
    //HGDIOBJ old_bitmap = SelectObject(memory_dc, memory_bitmap);
    GetObject(memory_bitmap, sizeof(BITMAP), &memory_bitmap_struct);
    Rectangle(memory_dc, 0, 0, 111, 222);
    GetObject(memory_bitmap, sizeof(BITMAP), &memory_bitmap_struct);
    GetObject(memory_dc, sizeof(BITMAP), &memory_bitmap_struct);


    BITMAPINFOHEADER bmiHeader = {};
    bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmiHeader.biWidth = window_width;
    bmiHeader.biHeight = -window_height;
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
        return 1;
    }




    POINT mouse_pointer_point;

    LARGE_INTEGER prev_pc_val;
    LARGE_INTEGER cur_pc_val;
    LONGLONG pc_diff;
    LONGLONG time_diff;
    QueryPerformanceCounter(&prev_pc_val);

    PAINTSTRUCT my_ps;
    while (g_Running)
    {
        
        while(message_read = PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {

            if (msg.message == WM_KEYUP || msg.message == WM_KEYDOWN) {
                HandleKeyboard(&msg);
            }
            if (msg.message == WM_QUIT) {
                g_Running = 0;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } 


        //GetCursorPos(&mouse_pointer_point); 
        int val1 = mouse_x % 256;//(mouse_pointer_point.x % 256);
        int val2 = mouse_y % 256;//(mouse_pointer_point.y % 256);

/*
        HBRUSH my_solid_brush2 = CreateSolidBrush(RGB(val1, val2, 128));
        HGDIOBJ old_brush = SelectObject(cur_hdc, my_solid_brush2);
        int x_left_top = mouse_x - 10;
        int y_left_top = mouse_y - 10;
        //char temp[256];
        //sprintf(temp, "(%d, %d)\n", mouse_x, mouse_y);
        //OutputDebugStringA(temp);

        //PatBlt(cur_hdc, x_left_top, y_left_top, 20, 20, PATCOPY);

        //PatBlt(cur_hdc, 0, 0, x_left_top, y_left_top, BLACKNESS);
        //Rectangle(cur_hdc, 0, 0, x_left_top, y_left_top);

        SelectObject(cur_hdc, old_brush);
        DeleteObject(my_solid_brush2);
*/

        DWORD color = 0;
        DWORD * cur_pixel = (DWORD *) bits;
        for (int index_row = 0; index_row < window_height; ++index_row)
        {
            for (int index_column = 0; index_column < window_width; ++index_column)
            {
                uint8 r = global_state.offset_y + index_row % 256;
                uint8 g = global_state.offset_x + index_column % 256;
                uint8 b = 0;
                color = ((((DWORD)r) << 16) | (((BYTE)g)<< 8) | b);
                *(cur_pixel++) = color;
            }

        }
        HGDIOBJ old_bitmap = SelectObject(memory_dc, DIBSection);
        BitBlt(display_dc, 0, 0, window_width, window_height, memory_dc, 0, 0, SRCCOPY);
        SelectObject(memory_dc, old_bitmap);

        QueryPerformanceCounter(&cur_pc_val);
        pc_diff = cur_pc_val.QuadPart - prev_pc_val.QuadPart;
        time_diff = pc_diff * 1'000'000;
        time_diff /= pc_freq.QuadPart;

        char temp[256];
        sprintf(temp, "%lld us\n", time_diff);
        OutputDebugStringA(temp);
        prev_pc_val = cur_pc_val;


        //OutputDebugStringA("All messages extracted.\n");
        /*if (message_read  < 0)
        {
            OutputDebugStringA("GetMessage got an error\n");
        }
        */
    }
    ClipCursor(NULL);
    return 0;
}

void RestrictCursor(HWND hwnd)
{
    RECT client_rect;
    GetClientRect(hwnd, &client_rect);
    POINT left_top = {client_rect.left, client_rect.top};
    POINT right_bottom = {client_rect.right, client_rect.bottom};
    ClientToScreen(hwnd, &left_top);
    ClientToScreen(hwnd, &right_bottom);
    RECT screen_rect;
    SetRect(&screen_rect, left_top.x, left_top.y, right_bottom.x, right_bottom.y);
    ClipCursor(&screen_rect);
}
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
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

        case WM_MOUSEMOVE:
        {
            mouse_x = GET_X_LPARAM(lParam);
            mouse_y = GET_Y_LPARAM(lParam);
        } break;

        case WM_SIZE:
        {
            //RestrictCursor(hwnd);
        } break;

        case WM_MOVE:
        {
            //RestrictCursor(hwnd);
        } break;


        default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    return 0;
}
