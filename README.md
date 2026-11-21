TinyGFX — Lightweight & Fast C++ Graphics Library

⚡ Small Footprint & High Speed: A minimal, fast, and extensible C++ graphics & input library inspired by Raylib — designed for pixel art, 8-bit tilesets (16×16), mouse drawing, and text-driven commands.

English | العربية

🇬🇧 English

✨ Overview

TinyGFX is a tiny, zero-bloat C++ graphics library built around a clean separation between a unified public API (tiny_gfx.hpp) and lightweight OS-specific backends (platform_*.cpp). It is ideal for interactive drawing tools, 16×16 tile/block editors, and quick 2D prototypes.

🚀 Key Features

Small & Fast: Minimal core focused purely on window management, pixel buffers, and input polling with low overhead.

Cross-Platform Architecture: Unified API layer decoupled from platform backends (Windows Win32, Linux X11, etc.).

Mouse & Text Input: Built-in mouse coordinates, button states, scroll wheel delta, and character input queue for command bars.

Extensible by Design: Modular extensions (such as 16×16 8-bit tilesets and 256-color palettes) plug cleanly on top of the core.

📁 Project Structure

project/
├── tiny_gfx.hpp          # Unified cross-platform API
├── platform_win32.cpp    # Windows backend (Win32 API)
├── platform_x11.cpp      # Linux backend (X11 - planned/optional)
├── ext_tileset.hpp       # 16x16 8-bit tileset & grid extension
├── main.cpp              # Example application (mouse + text drawing)
└── README.md             # Project documentation


💻 Quick Example

#include "tiny_gfx.hpp"

int main() {
    tgfx::InitWindow(800, 600, "TinyGFX Demo");
    tgfx::SetTargetFPS(60);

    while (!tgfx::WindowShouldClose()) {
        int mx = tgfx::GetMouseX();
        int my = tgfx::GetMouseY();

        tgfx::BeginDrawing();
        tgfx::ClearBackground({25, 25, 30, 255});

        if (tgfx::IsMouseDown(tgfx::MouseButton::Left)) {
            tgfx::DrawRect(mx - 8, my - 8, 16, 16, {255, 200, 50, 255});
        }

        tgfx::EndDrawing();
    }

    tgfx::CloseWindow();
    return 0;
}


🛠️ Build (Windows / MinGW)

g++ -std=c++17 -O2 main.cpp platform_win32.cpp -lgdi32 -luser32 -o tinygfx_demo.exe


🤝 Contributing

Contributions are welcome! Feel free to open an issue or submit a pull request for:

Adding or improving platform backends (Linux X11, macOS Cocoa, Wayland).

Optimizing software blitting and pixel primitives.

Expanding 8-bit tileset / map editor utilities.

🇸🇦 العربية

✨ نظرة عامة

TinyGFX هي مكتبة رسومية صغيرة الحجم وسريعة بلغة C++ مستوحاة من بساطة Raylib. تعتمد على فصل الواجهة البرمجية الموحدة (tiny_gfx.hpp) عن طبقة نظام التشغيل (platform_*.cpp)، مما يجعلها خفيفة، سريعة، وسهلة التوسع لبناء أدوات الرسم بالماوس والأوامر النصية وأنظمة البلوكات (16×16).

🚀 أبرز المزايا

صغيرة الحجم وسريعة: نواة خفيفة بدون تعقيد، تركّز على إدارة النافذة، رسم البكسلات، وقراءة المدخلات بأعلى سرعة.

معمارية قابلة للتوسع: واجهة موحدة مستقلة عن نظام التشغيل، مع إمكانية إضافة ميزات جديدة في ملفات منفصلة.

دعم الرسم بالماوس والكتابة: قراءة إحداثيات المؤشر وأزرار الماوس وعجلة التمرير، بالإضافة إلى التقاط الأحرف المكتوبة للأوامر النصية.

متوافقة مع البلوكات 8-Bit: جاهزة للربط مع بلوكات 16×16 ولوحات الألوان ذات 256 لوناً.

📁 هيكل المشروع

project/
├── tiny_gfx.hpp          # الواجهة البرمجية الموحدة
├── platform_win32.cpp    # طبقة التنفيذ لنظام Windows
├── platform_x11.cpp      # طبقة التنفيذ لنظام Linux (اختياري)
├── ext_tileset.hpp       # توسعة البلوكات 16x16 والشبكة
├── main.cpp              # مثال تطبيقي للرسم بالماوس والكتابة
└── README.md             # ملف التوثيق


🗺️ خارطة الطريق (Roadmap)

[ ] إتمام طبقة النظام لـ Windows (Win32) و Linux (X11).

[ ] ربط محرر البلوكات 16×16 مع حفظ وتحميل الملفات الثنائية.

[ ] إضافة شريط أوامر نصي متكامل داخل النافذة.

📄 License / الرخصة

MIT License — حرة للاستخدام والتعديل والمساهمة.
