#include "tiny_gfx.hpp"
#include <windows.h>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <algorithm>

namespace tgfx {

namespace {
HWND g_hwnd = nullptr;
int g_width = 0;
int g_height = 0;
bool g_should_close = false;
int g_target_fps = 60;

std::vector<uint32_t> g_framebuffer; // 0x00RRGGBB
BITMAPINFO g_bmi{};

int g_mouse_x = 0;
int g_mouse_y = 0;
int g_wheel_delta = 0;
bool g_mouse_down[3] = {false, false, false};
bool g_mouse_prev[3] = {false, false, false};

bool g_key_down[256] = {false};
bool g_key_prev[256] = {false};
std::queue<char> g_typed_chars;

std::chrono::steady_clock::time_point g_frame_start;

inline uint32_t PackColor(Color c) {
    return (static_cast<uint32_t>(c.r) << 16) |
           (static_cast<uint32_t>(c.g) << 8)  |
           static_cast<uint32_t>(c.b);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CLOSE:
        case WM_DESTROY:
            g_should_close = true;
            PostQuitMessage(0);
            return 0;
        case WM_MOUSEMOVE:
            g_mouse_x = static_cast<short>(LOWORD(lParam));
            g_mouse_y = static_cast<short>(HIWORD(lParam));
            return 0;
        case WM_MOUSEWHEEL:
            g_wheel_delta += GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;
            return 0;
        case WM_LBUTTONDOWN: g_mouse_down[0] = true; return 0;
        case WM_LBUTTONUP:   g_mouse_down[0] = false; return 0;
        case WM_RBUTTONDOWN: g_mouse_down[1] = true; return 0;
        case WM_RBUTTONUP:   g_mouse_down[1] = false; return 0;
        case WM_MBUTTONDOWN: g_mouse_down[2] = true; return 0;
        case WM_MBUTTONUP:   g_mouse_down[2] = false; return 0;
        case WM_KEYDOWN:
            if (wParam < 256) g_key_down[wParam] = true;
            return 0;
        case WM_KEYUP:
            if (wParam < 256) g_key_down[wParam] = false;
            return 0;
        case WM_CHAR:
            if (wParam >= 32 && wParam <= 126) {
                g_typed_chars.push(static_cast<char>(wParam));
            } else if (wParam == '\r') {
                g_typed_chars.push('\n');
            } else if (wParam == '\b') {
                g_typed_chars.push('\b');
            }
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wParam, lParam);
}
} // namespace

bool InitWindow(int width, int height, const std::string& title) {
    g_width = width;
    g_height = height;
    g_should_close = false;
    g_framebuffer.assign(width * height, 0);

    HINSTANCE hInstance = GetModuleHandleA(nullptr);
    WNDCLASSA wc{};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = "TinyGFXWindowClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassA(&wc);

    RECT rect{0, 0, width, height};
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX, FALSE);

    g_hwnd = CreateWindowA(
        wc.lpszClassName, title.c_str(),
        (WS_OVERLAPPEDWINDOW & ~WS_THICKFRAME & ~WS_MAXIMIZEBOX) | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        nullptr, nullptr, hInstance, nullptr
    );

    if (!g_hwnd) return false;

    g_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    g_bmi.bmiHeader.biWidth = width;
    g_bmi.bmiHeader.biHeight = -height; // من الأعلى إلى الأسفل
    g_bmi.bmiHeader.biPlanes = 1;
    g_bmi.bmiHeader.biBitCount = 32;
    g_bmi.bmiHeader.biCompression = BI_RGB;

    return true;
}

void CloseWindow() {
    if (g_hwnd) {
        DestroyWindow(g_hwnd);
        g_hwnd = nullptr;
    }
}

bool WindowShouldClose() {
    for (int i = 0; i < 3; ++i) g_mouse_prev[i] = g_mouse_down[i];
    for (int i = 0; i < 256; ++i) g_key_prev[i] = g_key_down[i];
    g_wheel_delta = 0;

    MSG msg;
    while (PeekMessageA(&msg, nullptr, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    return g_should_close;
}

void SetTargetFPS(int fps) { g_target_fps = (fps > 0) ? fps : 60; }

void BeginDrawing() {
    g_frame_start = std::chrono::steady_clock::now();
}

void EndDrawing() {
    if (g_hwnd) {
        HDC hdc = GetDC(g_hwnd);
        StretchDIBits(
            hdc, 0, 0, g_width, g_height,
            0, 0, g_width, g_height,
            g_framebuffer.data(), &g_bmi, DIB_RGB_COLORS, SRCCOPY
        );
        ReleaseDC(g_hwnd, hdc);
    }

    auto target_dur = std::chrono::milliseconds(1000 / g_target_fps);
    auto elapsed = std::chrono::steady_clock::now() - g_frame_start;
    if (elapsed < target_dur) {
        std::this_thread::sleep_for(target_dur - elapsed);
    }
}

void ClearBackground(Color color) {
    std::fill(g_framebuffer.begin(), g_framebuffer.end(), PackColor(color));
}

void DrawPixel(int x, int y, Color color) {
    if (x < 0 || x >= g_width || y < 0 || y >= g_height) return;
    g_framebuffer[y * g_width + x] = PackColor(color);
}

void DrawRect(int x, int y, int w, int h, Color color) {
    uint32_t c = PackColor(color);
    int x0 = std::max(0, x), y0 = std::max(0, y);
    int x1 = std::min(g_width, x + w), y1 = std::min(g_height, y + h);
    for (int py = y0; py < y1; ++py) {
        for (int px = x0; px < x1; ++px) {
            g_framebuffer[py * g_width + px] = c;
        }
    }
}

void DrawRectLines(int x, int y, int w, int h, Color color) {
    for (int i = 0; i < w; ++i) {
        DrawPixel(x + i, y, color);
        DrawPixel(x + i, y + h - 1, color);
    }
    for (int j = 0; j < h; ++j) {
        DrawPixel(x, y + j, color);
        DrawPixel(x + w - 1, y + j, color);
    }
}
namespace {
const uint8_t* GetGlyph5x7(char c) {
    static const uint8_t space[7] = {0,0,0,0,0,0,0};
    static const uint8_t colon[7] = {0,4,4,0,4,4,0};
    static const uint8_t dash[7]  = {0,0,0,14,0,0,0};
    static const uint8_t plus[7]  = {0,4,4,31,4,4,0};
    static const uint8_t dot[7]   = {0,0,0,0,0,4,0};
    static const uint8_t slash[7] = {1,2,4,4,8,16,0};
    static const uint8_t lparen[7]= {2,4,8,8,8,4,2};
    static const uint8_t rparen[7]= {8,4,2,2,2,4,8};
    static const uint8_t lt[7]    = {2,4,8,16,8,4,2};
    static const uint8_t gt[7]    = {16,8,4,2,4,8,16};
    static const uint8_t eq[7]    = {0,0,31,0,31,0,0};
    static const uint8_t under[7] = {0,0,0,0,0,0,31};

    static const uint8_t digits[10][7] = {
        {14,17,19,21,25,17,14}, // 0
        {4,12,4,4,4,4,14},     // 1
        {14,17,1,6,8,16,31},    // 2
        {30,1,1,14,1,1,30},     // 3
        {2,6,10,18,31,2,2},     // 4
        {31,16,30,1,1,17,14},   // 5
        {14,16,30,17,17,17,14}, // 6
        {31,1,2,4,8,8,8},       // 7
        {14,17,17,14,17,17,14}, // 8
        {14,17,17,15,1,1,14}    // 9
    };

    static const uint8_t alpha[26][7] = {
        {14,17,17,31,17,17,17}, // A
        {30,17,17,30,17,17,30}, // B
        {14,17,16,16,16,17,14}, // C
        {28,18,17,17,17,18,28}, // D
        {31,16,16,30,16,16,31}, // E
        {31,16,16,30,16,16,16}, // F
        {14,17,16,23,17,17,15}, // G
        {17,17,17,31,17,17,17}, // H
        {14,4,4,4,4,4,14},      // I
        {7,2,2,2,2,18,12},      // J
        {17,18,20,24,20,18,17}, // K
        {16,16,16,16,16,16,31}, // L
        {17,27,21,21,17,17,17}, // M
        {17,25,21,19,17,17,17}, // N
        {14,17,17,17,17,17,14}, // O
        {30,17,17,30,16,16,16}, // P
        {14,17,17,17,21,18,13}, // Q
        {30,17,17,30,20,18,17}, // R
        {15,16,16,14,1,1,30},   // S
        {31,4,4,4,4,4,4},       // T
        {17,17,17,17,17,17,14}, // U
        {17,17,17,17,17,10,4},  // V
        {17,17,17,21,21,27,17}, // W
        {17,17,10,4,10,17,17},  // X
        {17,17,10,4,4,4,4},     // Y
        {31,1,2,4,8,16,31}      // Z
    };

    if (c >= '0' && c <= '9') return digits[c - '0'];
    if (c >= 'a' && c <= 'z') return alpha[c - 'a'];
    if (c >= 'A' && c <= 'Z') return alpha[c - 'A'];
    switch (c) {
        case ':': return colon;
        case '-': return dash;
        case '+': return plus;
        case '.': return dot;
        case '/': return slash;
        case '(': return lparen;
        case ')': return rparen;
        case '<': return lt;
        case '>': return gt;
        case '=': return eq;
        case '_': return under;
        default:  return space;
    }
}
} // namespace

void DrawTextSimple(int x, int y, const std::string& text, Color color) {
    const int scale = 2;
    int cursor_x = x;
    for (char ch : text) {
        const uint8_t* glyph = GetGlyph5x7(ch);
        for (int row = 0; row < 7; ++row) {
            for (int col = 0; col < 5; ++col) {
                if (glyph[row] & (1 << (4 - col))) {
                    DrawRect(cursor_x + col * scale, y + row * scale, scale, scale, color);
                }
            }
        }
        cursor_x += 6 * scale;
    }
}

int GetMouseX() { return g_mouse_x; }
int GetMouseY() { return g_mouse_y; }
int GetMouseWheelDelta() { return g_wheel_delta; }
bool IsMouseDown(MouseButton btn) { return g_mouse_down[static_cast<int>(btn)]; }
bool IsMousePressed(MouseButton btn) {
    int i = static_cast<int>(btn);
    return g_mouse_down[i] && !g_mouse_prev[i];
}
bool IsKeyPressedSimple(int key_code) {
    return (key_code >= 0 && key_code < 256) && g_key_down[key_code] && !g_key_prev[key_code];
}
char PopTypedChar() {
    if (g_typed_chars.empty()) return '\0';
    char c = g_typed_chars.front();
    g_typed_chars.pop();
    return c;
}
void SendCommand(const std::string&) {}

} // namespace tgfx
