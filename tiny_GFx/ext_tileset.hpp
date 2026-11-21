#pragma once
#include "tiny_gfx.hpp"
#include <cstdint>
#include <cstring>

namespace tgfx {

constexpr int BLOCK_W = 16;
constexpr int BLOCK_H = 16;
constexpr int BLOCK_PIXELS = BLOCK_W * BLOCK_H;
constexpr int NUM_BLOCKS = 1000;
constexpr int PALETTE_SIZE = 256;

struct Tileset8Bit {
    Color palette[PALETTE_SIZE];
    uint8_t pixels[NUM_BLOCKS * BLOCK_PIXELS];
};

inline void InitDefaultPalette(Tileset8Bit& ts) {
    for (int i = 0; i < PALETTE_SIZE; ++i) {
        uint8_t r = static_cast<uint8_t>(((i * 53) + 40) % 256);
        uint8_t g = static_cast<uint8_t>(((i * 97) + 80) % 256);
        uint8_t b = static_cast<uint8_t>(((i * 151) + 120) % 256);
        if (i == 0) { r = 20; g = 22; b = 28; } // لون الخلفية الفارغة
        ts.palette[i] = Color{r, g, b, 255};
    }
}

inline void GenerateDemoBlocks(Tileset8Bit& ts) {
    std::memset(ts.pixels, 0, sizeof(ts.pixels));
    for (int id = 1; id < NUM_BLOCKS; ++id) {
        uint8_t main_col = static_cast<uint8_t>(((id * 37) % 255) + 1);
        uint8_t sec_col  = static_cast<uint8_t>(((id * 89 + 60) % 255) + 1);
        for (int y = 0; y < BLOCK_H; ++y) {
            for (int x = 0; x < BLOCK_W; ++x) {
                bool border = (x == 0 || y == 0 || x == BLOCK_W - 1 || y == BLOCK_H - 1);
                bool inner  = (x >= 4 && x <= 11 && y >= 4 && y <= 11);
                uint8_t idx = border ? sec_col : (inner ? main_col : static_cast<uint8_t>(((main_col + 25) % 255) + 1));
                ts.pixels[id * BLOCK_PIXELS + y * BLOCK_W + x] = idx;
            }
        }
    }
}

// تلوين بلوك كامل بلون محدد من اللوحة لتجربة تغيير الألوان فوراً
inline void SetBlockSolidColor(Tileset8Bit& ts, int block_id, uint8_t palette_idx) {
    if (block_id < 0 || block_id >= NUM_BLOCKS) return;
    std::memset(&ts.pixels[block_id * BLOCK_PIXELS], palette_idx, BLOCK_PIXELS);
}

inline void DrawBlock(const Tileset8Bit& ts, int block_id, int screen_x, int screen_y, int scale = 2) {
    if (block_id < 0 || block_id >= NUM_BLOCKS) return;
    const uint8_t* blk = &ts.pixels[block_id * BLOCK_PIXELS];
    for (int y = 0; y < BLOCK_H; ++y) {
        for (int x = 0; x < BLOCK_W; ++x) {
            Color c = ts.palette[blk[y * BLOCK_W + x]];
            DrawRect(screen_x + x * scale, screen_y + y * scale, scale, scale, c);
        }
    }
}

} // namespace tgfx
