#ifndef UNICODE
#define UNICODE
#endif

#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include <wingdi.h>


BOOL g_Running;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg,
                            WPARAM wParam, LPARAM lParam);

int mouse_x;
int mouse_y;

INT WINAPI wWinMain(HINSTANCE hInstance, 
                   HINSTANCE hPrevInstance,
                   PWSTR lpCmdLine,
                   INT nCmdShow)
{

    LARGE_INTEGER pc_freq;
    QueryPerformanceFrequency(&pc_freq);

    DWORD window_color = GetSysColor(COLOR_WINDOW);
    BYTE r_value = GetRValue(window_color);
    BYTE g_value = GetGValue(window_color);
    BYTE b_value = GetBValue(window_color);

    COLORREF my_color = RGB(0, 128, 255);
    INT sys_element = COLOR_WINDOW;
    SetSysColors(1, &sys_element, &my_color);


    window_color = GetSysColor(COLOR_WINDOW);
    r_value = GetRValue(window_color);
    g_value = GetGValue(window_color);
    b_value = GetBValue(window_color);


    //HBRUSH my_hatchbrush = CreateHatchBrush(HS_BDIAGONAL, my_color);
    HBRUSH my_solid_brush = CreateSolidBrush(my_color);

    const wchar_t CLASS_NAME[] = L"My Window Class";

    WNDCLASS wc = {};

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = my_solid_brush;
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

    HDC cur_hdc = GetDC(hwnd);
    //INT ret_set_stretch_blt_mode = SetStretchBltMode(cur_hdc, STRETCH_HALFTONE);
    //ReleaseDC(hwnd, cur_hdc);
    BOOL once = 1;

    //HDC main_window_display_dc = GetDC()
    //CreateCompatibleDC()

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
            if (msg.message == WM_QUIT) {
                g_Running = 0;
                break;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        } 

        if (once) 
        {
            HDC cur_hdc = GetDC(hwnd);
            INT cur_map_mod = GetMapMode(cur_hdc);
            INT device_num_colors = GetDeviceCaps(cur_hdc, NUMCOLORS);
            INT horz_size = GetDeviceCaps(cur_hdc, HORZSIZE);
            INT vert_size = GetDeviceCaps(cur_hdc, VERTSIZE);
            INT horz_res = GetDeviceCaps(cur_hdc, HORZRES);
            INT vert_res = GetDeviceCaps(cur_hdc, VERTRES);
            INT v_refresh = GetDeviceCaps(cur_hdc, VREFRESH);
            INT stretch_blt_mode = GetStretchBltMode(cur_hdc);
            once = 0;
        }

        //HDC cur_hdc = BeginPaint(hwnd, &my_ps);

        //GetCursorPos(&mouse_pointer_point); 
        int val1 = mouse_x % 256;//(mouse_pointer_point.x % 256);
        int val2 = mouse_y % 256;//(mouse_pointer_point.y % 256);

        HBRUSH my_solid_brush2 = CreateSolidBrush(RGB(val1, val2, 128));
        HGDIOBJ old_brush = SelectObject(cur_hdc, my_solid_brush2);
        int x_left_top = mouse_x - 10;
        int y_left_top = mouse_y - 10;
        //char temp[256];
        //sprintf(temp, "(%d, %d)\n", mouse_x, mouse_y);
        //OutputDebugStringA(temp);
        PatBlt(cur_hdc, x_left_top, y_left_top, 20, 20, PATCOPY);
        //PatBlt(cur_hdc, 0, 0, x_left_top, y_left_top, BLACKNESS);
        //Rectangle(cur_hdc, 0, 0, x_left_top, y_left_top);

        SelectObject(cur_hdc, old_brush);
        DeleteObject(my_solid_brush2);

        //EndPaint(hwnd, &my_ps);
        //ReleaseDC(hwnd, cur_hdc);



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
            RestrictCursor(hwnd);
        } break;

        case WM_MOVE:
        {
            RestrictCursor(hwnd);
        } break;

        case WM_KEYDOWN:
        {
            switch(wParam)
            {
                case 0x57: // 'w'
                {
                    OutputDebugStringA("w: down\n");
                } break;

                case 0x41: // 'a'
                {
                    OutputDebugStringA("a: down\n");
                } break;

                case 0x53: // 's'
                {
                    OutputDebugStringA("s: down\n");
                } break;

                case 0x44: // 'd'
                {
                    OutputDebugStringA("d: down\n");
                } break;

                default:
                {
                    return DefWindowProc(hwnd, uMsg, wParam, lParam);
                } break;
            }

        } break;

        case WM_KEYUP:
        {
            switch(wParam)
            {

                case 0x57: // 'w'
                {
                    OutputDebugStringA("w: up\n");
                } break;

                case 0x41: // 'a'
                {
                    OutputDebugStringA("a: up\n");
                } break;

                case 0x53: // 's'
                {
                    OutputDebugStringA("s: up\n");
                } break;

                case 0x44: // 'd'
                {
                    OutputDebugStringA("d: up\n");
                } break;

                default:
                {
                    return DefWindowProc(hwnd, uMsg, wParam, lParam);
                } break;
            }
        } break;

        default:
        {
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
        } break;
    }
    return 0;
}
