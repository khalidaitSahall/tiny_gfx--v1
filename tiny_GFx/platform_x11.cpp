#include "tiny_gfx.hpp"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <vector>
#include <queue>
#include <chrono>
#include <thread>
#include <algorithm>

namespace tgfx {

namespace {
Display* g_display = nullptr;
Window g_window = 0;
GC g_gc = nullptr;
XImage* g_ximage = nullptr;
Atom g_wm_delete = 0;

int g_width = 0;
int g_height = 0;
bool g_should_close = false;
int g_target_fps = 60;

std::vector<uint32_t> g_framebuffer;

int g_mouse_x = 0;
int g_mouse_y = 0;
int g_wheel_delta = 0;
bool g_mouse_down[3] = {false, false, false};
bool g_mouse_prev[3] = {false, false, false};
std::queue<char> g_typed_chars;

std::chrono::steady_clock::time_point g_frame_start;

inline uint32_t PackColor(Color c) {
    return (static_cast<uint32_t>(c.r) << 16) |
           (static_cast<uint32_t>(c.g) << 8)  |
           static_cast<uint32_t>(c.b);
}
} // namespace

bool InitWindow(int width, int height, const std::string& title) {
    g_width = width;
    g_height = height;
    g_should_close = false;
    g_framebuffer.assign(width * height, 0);

    g_display = XOpenDisplay(nullptr);
    if (!g_display) return false;

    int screen = DefaultScreen(g_display);
    g_window = XCreateSimpleWindow(
        g_display, RootWindow(g_display, screen),
        100, 100, width, height, 1,
        BlackPixel(g_display, screen), WhitePixel(g_display, screen)
    );

    XStoreName(g_display, g_window, title.c_str());
    XSelectInput(g_display, g_window,
        ExposureMask | KeyPressMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);

    g_wm_delete = XInternAtom(g_display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(g_display, g_window, &g_wm_delete, 1);

    g_gc = XCreateGC(g_display, g_window, 0, nullptr);
    Visual* visual = DefaultVisual(g_display, screen);
    g_ximage = XCreateImage(
        g_display, visual, DefaultDepth(g_display, screen),
        ZPixmap, 0, reinterpret_cast<char*>(g_framebuffer.data()),
        width, height, 32, 0
    );

    XMapWindow(g_display, g_window);
    XFlush(g_display);
    return true;
}

void CloseWindow() {
    if (g_ximage) {
        g_ximage->data = nullptr; // تجنب تحرير ذاكرة std::vector مرتين
        XDestroyImage(g_ximage);
        g_ximage = nullptr;
    }
    if (g_display) {
        if (g_gc) XFreeGC(g_display, g_gc);
        if (g_window) XDestroyWindow(g_display, g_window);
        XCloseDisplay(g_display);
        g_display = nullptr;
    }
}

bool WindowShouldClose() {
    for (int i = 0; i < 3; ++i) g_mouse_prev[i] = g_mouse_down[i];
    g_wheel_delta = 0;

    while (g_display && XPending(g_display) > 0) {
        XEvent ev;
        XNextEvent(g_display, &ev);
        switch (ev.type) {
            case ClientMessage:
                if (static_cast<Atom>(ev.xclient.data.l[0]) == g_wm_delete) g_should_close = true;
                break;
            case MotionNotify:
                g_mouse_x = ev.xmotion.x;
                g_mouse_y = ev.xmotion.y;
                break;
            case ButtonPress:
                if (ev.xbutton.button == Button1) g_mouse_down[0] = true;
                else if (ev.xbutton.button == Button3) g_mouse_down[1] = true;
                else if (ev.xbutton.button == Button2) g_mouse_down[2] = true;
                else if (ev.xbutton.button == Button4) g_wheel_delta += 1;
                else if (ev.xbutton.button == Button5) g_wheel_delta -= 1;
                break;
            case ButtonRelease:
                if (ev.xbutton.button == Button1) g_mouse_down[0] = false;
                else if (ev.xbutton.button == Button3) g_mouse_down[1] = false;
                else if (ev.xbutton.button == Button2) g_mouse_down[2] = false;
                break;
            case KeyPress: {
                char buf[8] = {0};
                KeySym ks;
                int len = XLookupString(&ev.xkey, buf, sizeof(buf), &ks, nullptr);
                if (ks == XK_Return) g_typed_chars.push('\n');
                else if (ks == XK_BackSpace) g_typed_chars.push('\b');
                else if (len > 0 && buf[0] >= 32 && buf[0] <= 126) g_typed_chars.push(buf[0]);
                break;
            }
        }
    }
    return g_should_close;
}

void SetTargetFPS(int fps) { g_target_fps = (fps > 0) ? fps : 60; }
void BeginDrawing() { g_frame_start = std::chrono::steady_clock::now(); }

void EndDrawing() {
    if (g_display && g_ximage) {
        XPutImage(g_display, g_window, g_gc, g_ximage, 0, 0, 0, 0, g_width, g_height);
        XFlush(g_display);
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

void DrawTextSimple(int x, int y, const std::string& text, Color) {
    if (g_display && g_window) {
        XDrawString(g_display, g_window, g_gc, x, y + 12, text.c_str(), static_cast<int>(text.size()));
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
bool IsKeyPressedSimple(int) { return false; }
char PopTypedChar() {
    if (g_typed_chars.empty()) return '\0';
    char c = g_typed_chars.front();
    g_typed_chars.pop();
    return c;
}
void SendCommand(const std::string&) {}

} // namespace tgfx
