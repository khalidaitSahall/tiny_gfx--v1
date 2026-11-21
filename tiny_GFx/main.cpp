#include "tiny_gfx.hpp"
#include "ext_tileset.hpp"
#include <sstream>
#include <string>

void ExecuteCommand(const std::string& cmd, int& active_block, int map[16][16]) {
    std::istringstream iss(cmd);
    std::string op;
    iss >> op;

    if (op == "brush") {
        int id;
        if (iss >> id && id >= 0 && id < tgfx::NUM_BLOCKS) active_block = id;
    } else if (op == "set") {
        int x, y, id;
        if (iss >> x >> y >> id && x >= 0 && x < 16 && y >= 0 && y < 16 && id >= 0 && id < tgfx::NUM_BLOCKS) {
            map[y][x] = id;
        }
    } else if (op == "fill") {
        int id;
        if (iss >> id && id >= 0 && id < tgfx::NUM_BLOCKS) {
            for (int y = 0; y < 16; ++y)
                for (int x = 0; x < 16; ++x)
                    map[y][x] = id;
        }
    } else if (op == "clear") {
        for (int y = 0; y < 16; ++y)
            for (int x = 0; x < 16; ++x)
                map[y][x] = 0;
    }
}

int main() {
    tgfx::Tileset8Bit ts{};
    tgfx::InitDefaultPalette(ts);
    tgfx::GenerateDemoBlocks(ts);

    tgfx::InitWindow(820, 560, "TinyGFX - Mouse & Text Block Editor");
    tgfx::SetTargetFPS(60);

    int map[16][16] = {0};
    int active_block = 12;
    const int scale = 2;
    const int tile_px = tgfx::BLOCK_W * scale; // 32px

    std::string command_input;

    while (!tgfx::WindowShouldClose()) {
        // تغيير البلوك بعجلة الماوس
        active_block += tgfx::GetMouseWheelDelta();
        if (active_block < 0) active_block = 0;
        if (active_block >= tgfx::NUM_BLOCKS) active_block = tgfx::NUM_BLOCKS - 1;

        // استقبال الكتابة المباشرة للأوامر النصية
        char ch;
        while ((ch = tgfx::PopTypedChar()) != '\0') {
            if (ch == '\n') {
                if (!command_input.empty()) {
                    ExecuteCommand(command_input, active_block, map);
                    command_input.clear();
                }
            } else if (ch == '\b') {
                if (!command_input.empty()) command_input.pop_back();
            } else {
                command_input.push_back(ch);
            }
        }

        // الرسم بالماوس على الشبكة (16x16)
        int gx = (tgfx::GetMouseX() - 20) / tile_px;
        int gy = (tgfx::GetMouseY() - 20) / tile_px;
        if (gx >= 0 && gx < 16 && gy >= 0 && gy < 16) {
            if (tgfx::IsMouseDown(tgfx::MouseButton::Left)) map[gy][gx] = active_block;
            if (tgfx::IsMouseDown(tgfx::MouseButton::Right)) map[gy][gx] = 0;
        }

        tgfx::BeginDrawing();
        tgfx::ClearBackground({28, 30, 36, 255});

        // رسم الشبكة
        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int sx = 20 + x * tile_px;
                int sy = 20 + y * tile_px;
                tgfx::DrawBlock(ts, map[y][x], sx, sy, scale);
                tgfx::DrawRectLines(sx, sy, tile_px, tile_px, {60, 65, 75, 255});
            }
        }

        // معاينة البلوك النشط وشريط الأوامر
        tgfx::DrawBlock(ts, active_block, 560, 40, 4);
        tgfx::DrawRectLines(560, 40, 64, 64, {255, 255, 255, 255});

        tgfx::DrawRect(560, 140, 230, 28, {15, 17, 20, 255});
        tgfx::DrawRectLines(560, 140, 230, 28, {100, 180, 255, 255});

        tgfx::EndDrawing();

        // رسم النصوص فوق الإطار
        tgfx::DrawTextSimple(560, 18, "Block ID: " + std::to_string(active_block), {240, 240, 240, 255});
        tgfx::DrawTextSimple(560, 118, "Cmd (brush/set/fill/clear):", {180, 200, 220, 255});
        tgfx::DrawTextSimple(566, 146, "> " + command_input, {120, 255, 140, 255});
    }

    tgfx::CloseWindow();
    return 0;
}
