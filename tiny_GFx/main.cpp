#include "tiny_gfx.hpp"
#include "ext_tileset.hpp"
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

bool SaveAsPPM(const std::string& filename, const tgfx::Tileset8Bit& ts, const int map[16][16], std::string& err, int scale = 2) {
    const int grid_w = 16;
    const int grid_h = 16;
    const int img_w = grid_w * tgfx::BLOCK_W * scale;
    const int img_h = grid_h * tgfx::BLOCK_H * scale;

    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        err = "Cannot open: " + filename;
        std::cerr << err << "\n";
        return false;
    }

    out << "P6\n" << img_w << " " << img_h << "\n255\n";

    for (int py = 0; py < img_h; ++py) {
        int gy = py / (tgfx::BLOCK_H * scale);
        int by = (py / scale) % tgfx::BLOCK_H;
        for (int px = 0; px < img_w; ++px) {
            int gx = px / (tgfx::BLOCK_W * scale);
            int bx = (px / scale) % tgfx::BLOCK_W;

            int block_id = map[gy][gx];
            if (block_id < 0 || block_id >= tgfx::NUM_BLOCKS) block_id = 0;
            uint8_t pal_idx = ts.pixels[block_id * tgfx::BLOCK_PIXELS + by * tgfx::BLOCK_W + bx];
            tgfx::Color c = ts.palette[pal_idx];

            out.put(static_cast<char>(c.r));
            out.put(static_cast<char>(c.g));
            out.put(static_cast<char>(c.b));
        }
    }

    out.flush();
    if (!out.good()) {
        err = "Write error: " + filename;
        std::cerr << err << "\n";
        return false;
    }
    return true;
}

bool SaveAsBMP(const std::string& filename, const tgfx::Tileset8Bit& ts, const int map[16][16], std::string& err, int scale = 2) {
    const int width = 16 * tgfx::BLOCK_W * scale;
    const int height = 16 * tgfx::BLOCK_H * scale;
    const int row_padded = ((width * 3) + 3) & ~3;
    const int pixel_data_size = row_padded * height;
    const int file_size = 54 + pixel_data_size;

    std::ofstream out(filename, std::ios::binary);
    if (!out.is_open()) {
        err = "Cannot open: " + filename;
        std::cerr << err << "\n";
        return false;
    }

    uint8_t header[54] = {0};
    header[0] = 'B'; header[1] = 'M';
    header[2] = static_cast<uint8_t>(file_size);
    header[3] = static_cast<uint8_t>(file_size >> 8);
    header[4] = static_cast<uint8_t>(file_size >> 16);
    header[5] = static_cast<uint8_t>(file_size >> 24);
    header[10] = 54;

    header[14] = 40;
    header[18] = static_cast<uint8_t>(width);
    header[19] = static_cast<uint8_t>(width >> 8);
    header[20] = static_cast<uint8_t>(width >> 16);
    header[21] = static_cast<uint8_t>(width >> 24);
    header[22] = static_cast<uint8_t>(height);
    header[23] = static_cast<uint8_t>(height >> 8);
    header[24] = static_cast<uint8_t>(height >> 16);
    header[25] = static_cast<uint8_t>(height >> 24);
    header[26] = 1;
    header[28] = 24;

    out.write(reinterpret_cast<const char*>(header), 54);

    uint8_t pad[3] = {0, 0, 0};
    int pad_bytes = row_padded - width * 3;

    for (int y = height - 1; y >= 0; --y) {
        int gy = y / (tgfx::BLOCK_H * scale);
        int by = (y / scale) % tgfx::BLOCK_H;
        for (int x = 0; x < width; ++x) {
            int gx = x / (tgfx::BLOCK_W * scale);
            int bx = (x / scale) % tgfx::BLOCK_W;

            int block_id = map[gy][gx];
            if (block_id < 0 || block_id >= tgfx::NUM_BLOCKS) block_id = 0;
            uint8_t pal_idx = ts.pixels[block_id * tgfx::BLOCK_PIXELS + by * tgfx::BLOCK_W + bx];
            tgfx::Color c = ts.palette[pal_idx];

            uint8_t bgr[3] = {c.b, c.g, c.r};
            out.write(reinterpret_cast<const char*>(bgr), 3);
        }
        if (pad_bytes > 0) {
            out.write(reinterpret_cast<const char*>(pad), pad_bytes);
        }
    }

    out.flush();
    if (!out.good()) {
        err = "Write error: " + filename;
        std::cerr << err << "\n";
        return false;
    }
    return true;
}

void ExecuteCommand(const std::string& cmd, tgfx::Tileset8Bit& ts, int& active_block, int map[16][16], std::string& status_msg) {
    std::istringstream iss(cmd);
    std::string op;
    iss >> op;

    if (op == "brush" || op == "b") {
        int id;
        if (iss >> id && id >= 0 && id < tgfx::NUM_BLOCKS) {
            active_block = id;
            status_msg = "Brush: " + std::to_string(id);
        }
    } else if (op == "set" || op == "s") {
        int x, y, id;
        if (iss >> x >> y >> id && x >= 0 && x < 16 && y >= 0 && y < 16 && id >= 0 && id < tgfx::NUM_BLOCKS) {
            map[y][x] = id;
            status_msg = "Set (" + std::to_string(x) + "," + std::to_string(y) + ")=" + std::to_string(id);
        }
    } else if (op == "fill" || op == "f") {
        int id;
        if (iss >> id && id >= 0 && id < tgfx::NUM_BLOCKS) {
            for (int y = 0; y < 16; ++y)
                for (int x = 0; x < 16; ++x)
                    map[y][x] = id;
            status_msg = "Filled: " + std::to_string(id);
        }
    } else if (op == "clear" || op == "x") {
        for (int y = 0; y < 16; ++y)
            for (int x = 0; x < 16; ++x)
                map[y][x] = 0;
        status_msg = "Cleared";
    } else if (op == "paint" || op == "p") {
        int pal_idx;
        if (iss >> pal_idx && pal_idx >= 0 && pal_idx < tgfx::PALETTE_SIZE) {
            tgfx::SetBlockSolidColor(ts, active_block, static_cast<uint8_t>(pal_idx));
            status_msg = "Paint pal: " + std::to_string(pal_idx);
        }
    } else if (op == "rgb" || op == "c") {
        int r, g, b;
        if (iss >> r >> g >> b) {
            uint8_t idx = ts.pixels[active_block * tgfx::BLOCK_PIXELS];
            ts.palette[idx] = tgfx::Color{
                static_cast<uint8_t>(r),
                static_cast<uint8_t>(g),
                static_cast<uint8_t>(b),
                255
            };
            status_msg = "RGB updated";
        }
    } else if (op == "bmp" || op == "m") {
        std::string fname, err;
        if (!(iss >> fname)) fname = "output.bmp";
        status_msg = SaveAsBMP(fname, ts, map, err) ? ("Saved: " + fname) : err;
    } else if (op == "ppm" || op == "i") {
        std::string fname, err;
        if (!(iss >> fname)) fname = "output.ppm";
        status_msg = SaveAsPPM(fname, ts, map, err) ? ("Saved: " + fname) : err;
    }
}

int main() {
    tgfx::Tileset8Bit ts{};
    tgfx::InitDefaultPalette(ts);
    tgfx::GenerateDemoBlocks(ts);

    tgfx::InitWindow(840, 560, "TinyGFX - Mouse & Text Block Editor");
    tgfx::SetTargetFPS(60);

    int map[16][16] = {0};
    int active_block = 12;
    const int scale = 2;
    const int tile_px = tgfx::BLOCK_W * scale;

    std::string command_input;
    std::string status_msg = "Ready";

    while (!tgfx::WindowShouldClose()) {
        active_block += tgfx::GetMouseWheelDelta();
        if (active_block < 0) active_block = 0;
        if (active_block >= tgfx::NUM_BLOCKS) active_block = tgfx::NUM_BLOCKS - 1;

        char ch;
        while ((ch = tgfx::PopTypedChar()) != '\0') {
            if (ch == '\n') {
                if (!command_input.empty()) {
                    ExecuteCommand(command_input, ts, active_block, map, status_msg);
                    command_input.clear();
                }
            } else if (ch == '\b') {
                if (!command_input.empty()) command_input.pop_back();
            } else {
                command_input.push_back(ch);
            }
        }

        int gx = (tgfx::GetMouseX() - 20) / tile_px;
        int gy = (tgfx::GetMouseY() - 20) / tile_px;
        if (gx >= 0 && gx < 16 && gy >= 0 && gy < 16) {
            if (tgfx::IsMouseDown(tgfx::MouseButton::Left)) map[gy][gx] = active_block;
            if (tgfx::IsMouseDown(tgfx::MouseButton::Right)) map[gy][gx] = 0;
        }

        tgfx::BeginDrawing();
        tgfx::ClearBackground({28, 30, 36, 255});

        for (int y = 0; y < 16; ++y) {
            for (int x = 0; x < 16; ++x) {
                int sx = 20 + x * tile_px;
                int sy = 20 + y * tile_px;
                tgfx::DrawBlock(ts, map[y][x], sx, sy, scale);
                tgfx::DrawRectLines(sx, sy, tile_px, tile_px, {60, 65, 75, 255});
            }
        }

        // معاينة البلوك النشط
        tgfx::DrawBlock(ts, active_block, 560, 40, 4);
        tgfx::DrawRectLines(560, 40, 64, 64, {255, 255, 255, 255});
        tgfx::DrawTextSimple(636, 56, "ID: " + std::to_string(active_block), {240, 240, 240, 255});

        // خانة إدخال الأوامر
        tgfx::DrawTextSimple(560, 114, "Cmd (Enter):", {180, 200, 220, 255});
        tgfx::DrawRect(560, 134, 265, 28, {15, 17, 20, 255});
        tgfx::DrawRectLines(560, 134, 265, 28, {100, 180, 255, 255});
        tgfx::DrawTextSimple(566, 141, "> " + command_input + "_", {120, 255, 140, 255});

        // خانة الحالة
        tgfx::DrawTextSimple(560, 172, "Status: " + status_msg, {255, 220, 100, 255});

        // خانة توضيحية ثابتة أسفل الأوامر تشرح كل أمر
        tgfx::DrawRect(560, 202, 265, 228, {20, 23, 29, 255});
        tgfx::DrawRectLines(560, 202, 265, 228, {75, 85, 105, 255});

        tgfx::DrawTextSimple(568, 210, "Commands Guide:", {130, 200, 255, 255});
        tgfx::DrawTextSimple(568, 232, "b id : pick block", {220, 225, 230, 255});
        tgfx::DrawTextSimple(568, 254, "s x y id : set cell", {220, 225, 230, 255});
        tgfx::DrawTextSimple(568, 276, "f id : fill grid", {220, 225, 230, 255});
        tgfx::DrawTextSimple(568, 298, "x : clear grid", {220, 225, 230, 255});
        tgfx::DrawTextSimple(568, 320, "p n : paint palette", {220, 225, 230, 255});
        tgfx::DrawTextSimple(568, 342, "c r g b : set RGB", {220, 225, 230, 255});
        tgfx::DrawTextSimple(568, 364, "m file : save BMP", {130, 255, 160, 255});
        tgfx::DrawTextSimple(568, 386, "i file : save PPM", {130, 255, 160, 255});

        tgfx::EndDrawing();
    }

    tgfx::CloseWindow();
    return 0;
}
