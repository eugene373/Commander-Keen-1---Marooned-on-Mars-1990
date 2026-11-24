/* Native Windows (x64) platform layer: Win32 window + GDI present, keyboard, QueryPerformanceCounter, PlaySound.
 * No third-party libraries: links winmm, gdi32, user32. */
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>
#include <string.h>
#include <stdio.h>
#include "ck1.h"

static HWND     g_hwnd;
static int      g_quit;
static uint8_t  g_down[256], g_latched[256];
static int      g_last_key;
static int      g_fullscreen;
static WINDOWPLACEMENT g_wp = { sizeof(WINDOWPLACEMENT) };
static LARGE_INTEGER g_freq, g_t0;
static char     g_exe_dir[MAX_PATH];

static void toggle_fullscreen(void)
{
    DWORD style = (DWORD)GetWindowLongPtr(g_hwnd, GWL_STYLE);
    if (!g_fullscreen) {
        MONITORINFO mi = { sizeof mi };
        if (GetWindowPlacement(g_hwnd, &g_wp) && GetMonitorInfo(MonitorFromWindow(g_hwnd, MONITOR_DEFAULTTOPRIMARY), &mi)) {
            SetWindowLongPtr(g_hwnd, GWL_STYLE, style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(g_hwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right - mi.rcMonitor.left,
                         mi.rcMonitor.bottom - mi.rcMonitor.top, SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
            g_fullscreen = 1;
        }
    } else {
        SetWindowLongPtr(g_hwnd, GWL_STYLE, style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(g_hwnd, &g_wp);
        SetWindowPos(g_hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        g_fullscreen = 0;
    }
}

static LRESULT CALLBACK wndproc(HWND h, UINT m, WPARAM w, LPARAM l)
{
    switch (m) {
    case WM_CLOSE: g_quit = 1; return 0;
    case WM_DESTROY: PostQuitMessage(0); return 0;
    case WM_ERASEBKGND: return 1;
    case WM_KILLFOCUS: memset(g_down, 0, sizeof g_down); memset(g_latched, 0, sizeof g_latched); return 0;
    case WM_SYSCOMMAND:
        if ((w & 0xFFF0) == SC_KEYMENU) return 0;       /* Alt alone must not open the (non-existent) menu: Alt = pogo */
        break;
    case WM_SYSKEYDOWN: case WM_KEYDOWN: {
        int vk = (int)(w & 0xFF);
        int repeat = (l >> 30) & 1;
        if (vk == VK_RETURN && (m == WM_SYSKEYDOWN) && !repeat) { toggle_fullscreen(); return 0; }
        if (vk == VK_F11 && !repeat) { toggle_fullscreen(); return 0; }
        g_down[vk] = 1;
        if (!repeat) {
            g_latched[vk] = 1;
            if ((vk >= 'A' && vk <= 'Z') || (vk >= '0' && vk <= '9') || vk == VK_RETURN || vk == VK_ESCAPE || vk == VK_SPACE) g_last_key = vk;
        }
        if (m == WM_SYSKEYDOWN) return 0;
        break;
    }
    case WM_SYSKEYUP: case WM_KEYUP:
        g_down[w & 0xFF] = 0;
        if (m == WM_SYSKEYUP) return 0;
        break;
    case WM_SYSCHAR: return 0;
    }
    return DefWindowProc(h, m, w, l);
}

int plat_init(const char *title)
{
    WNDCLASSA wc;
    RECT r = { 0, 0, 960, 720 };
    DWORD style = WS_OVERLAPPEDWINDOW;
    char *slash;
    QueryPerformanceFrequency(&g_freq);
    QueryPerformanceCounter(&g_t0);
    timeBeginPeriod(1);
    GetModuleFileNameA(NULL, g_exe_dir, MAX_PATH);
    slash = strrchr(g_exe_dir, '\\');
    if (slash) *slash = 0;
    memset(&wc, 0, sizeof wc);
    wc.lpfnWndProc = wndproc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = "Keen1Win64";
    if (!RegisterClassA(&wc)) return 0;
    AdjustWindowRect(&r, style, FALSE);
    g_hwnd = CreateWindowA(wc.lpszClassName, title, style, CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r.top,
                           NULL, NULL, wc.hInstance, NULL);
    if (!g_hwnd) return 0;
    ShowWindow(g_hwnd, SW_SHOW);
    return 1;
}

void plat_shutdown(void)
{
    PlaySoundA(NULL, NULL, 0);
    timeEndPeriod(1);
    if (g_hwnd) DestroyWindow(g_hwnd);
}

static int key(int vk) { return g_down[vk] || g_latched[vk]; }

int plat_poll(Input *in)
{
    MSG msg;
    int x, y, i;
    memset(g_latched, 0, sizeof g_latched);         /* latches from the previous poll are consumed below */
    g_last_key = 0;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (msg.message == WM_QUIT) g_quit = 1;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    in->quit = g_quit;
    for (i = 0; i < 256; i++) in->held[i] = (uint8_t)key(i);
    in->key = g_last_key;

    x = (key(VK_RIGHT) || key(VK_NUMPAD6)) - (key(VK_LEFT) || key(VK_NUMPAD4));
    y = (key(VK_DOWN) || key(VK_NUMPAD2)) - (key(VK_UP) || key(VK_NUMPAD8));
    if (key(VK_HOME) || key(VK_NUMPAD7))  { x -= 1; y -= 1; }
    if (key(VK_PRIOR) || key(VK_NUMPAD9)) { x += 1; y -= 1; }
    if (key(VK_END) || key(VK_NUMPAD1))   { x -= 1; y += 1; }
    if (key(VK_NEXT) || key(VK_NUMPAD3))  { x += 1; y += 1; }
    x = x > 0 ? 1 : (x < 0 ? -1 : 0);
    y = y > 0 ? 1 : (y < 0 ? -1 : 0);
    {
        static const int map[3][3] = { {7, 0, 1}, {6, 8, 2}, {5, 4, 3} };      /* [y+1][x+1] */
        in->dir = map[y + 1][x + 1];
    }
    in->jump = key(VK_CONTROL) || key('Z') || key(VK_SPACE);
    in->pogo = key(VK_MENU) || key('X') || key(VK_SPACE);       /* Space = jump + pogo = fire */
    return !g_quit;
}

void plat_present(const uint32_t *xrgb)
{
    RECT rc;
    HDC dc;
    BITMAPINFO bi;
    int cw, ch, dw, dh, dx, dy;
    if (!g_hwnd) return;
    GetClientRect(g_hwnd, &rc);
    cw = rc.right - rc.left; ch = rc.bottom - rc.top;
    if (cw <= 0 || ch <= 0) return;
    /* 320x200 pixels are shown with the 4:3 aspect of a real EGA/VGA monitor */
    dw = cw; dh = cw * 3 / 4;
    if (dh > ch) { dh = ch; dw = ch * 4 / 3; }
    dx = (cw - dw) / 2; dy = (ch - dh) / 2;
    dc = GetDC(g_hwnd);
    memset(&bi, 0, sizeof bi);
    bi.bmiHeader.biSize = sizeof bi.bmiHeader;
    bi.bmiHeader.biWidth = SCR_W;
    bi.bmiHeader.biHeight = -SCR_H;
    bi.bmiHeader.biPlanes = 1;
    bi.bmiHeader.biBitCount = 32;
    bi.bmiHeader.biCompression = BI_RGB;
    if (dx > 0 || dy > 0) PatBlt(dc, 0, 0, cw, ch, BLACKNESS);
    SetStretchBltMode(dc, COLORONCOLOR);
    StretchDIBits(dc, dx, dy, dw, dh, 0, 0, SCR_W, SCR_H, xrgb, &bi, DIB_RGB_COLORS, SRCCOPY);
    ReleaseDC(g_hwnd, dc);
}

int64_t plat_time_us(void)
{
    LARGE_INTEGER t;
    QueryPerformanceCounter(&t);
    return (int64_t)((t.QuadPart - g_t0.QuadPart) * 1000000LL / g_freq.QuadPart);
}

void plat_sleep_ms(int ms) { Sleep((DWORD)(ms < 0 ? 0 : ms)); }

void plat_play_wav(const uint8_t *wav, size_t len)
{
    (void)len;
    PlaySoundA((LPCSTR)wav, NULL, SND_MEMORY | SND_ASYNC | SND_NODEFAULT);
}

const char *plat_exe_dir(void) { return g_exe_dir; }

void plat_error(const char *msg)
{
    fprintf(stderr, "%s\n", msg);
    MessageBoxA(g_hwnd, msg, "Commander Keen 1", MB_OK | MB_ICONERROR);
}
