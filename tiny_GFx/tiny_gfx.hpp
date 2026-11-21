#pragma once
#include <cstdint>
#include <string>

namespace tgfx {

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

enum class MouseButton {
    Left = 0,
    Right = 1,
    Middle = 2
};

// 1. إدارة النافذة وحلقة العمل
bool InitWindow(int width, int height, const std::string& title);
void CloseWindow();
bool WindowShouldClose();
void SetTargetFPS(int fps);

// 2. دورة الرسم على الشاشة
void BeginDrawing();
void EndDrawing();
void ClearBackground(Color color);

// 3. دوال الرسم الأساسية (نقطة انطلاق للتوسع)
void DrawPixel(int x, int y, Color color);
void DrawRect(int x, int y, int w, int h, Color color);
void DrawRectLines(int x, int y, int w, int h, Color color);
void DrawTextSimple(int x, int y, const std::string& text, Color color);

// 4. قراءة المدخلات (الماوس + الكتابة)
int GetMouseX();
int GetMouseY();
int GetMouseWheelDelta();
bool IsMouseDown(MouseButton btn);
bool IsMousePressed(MouseButton btn);

bool IsKeyPressedSimple(int key_code);
char PopTypedChar(); // يرجع حرفاً مكتوباً للأوامر النصية، أو '\0' إذا لا يوجد

// 5. نقطة توسع اختيارية (مثلاً تمرير أمر نصي أو بيانات مخصصة)
void SendCommand(const std::string& cmd);

} // namespace tgfx
