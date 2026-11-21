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
        uint8_t r = static_cast<uint8_t>(((i >> 5) & 0x07) * 36);
        uint8_t g = static_cast<uint8_t>(((i >> 2) & 0x07) * 36);
        uint8_t b = static_cast<uint8_t>((i & 0x03) * 85);
        ts.palette[i] = Color{r, g, b, 255};
    }
}

inline void GenerateDemoBlocks(Tileset8Bit& ts) {
    std::memset(ts.pixels, 0, sizeof(ts.pixels));
    for (int id = 1; id < NUM_BLOCKS; ++id) {
        uint8_t c1 = static_cast<uint8_t>(id % 256);
        uint8_t c2 = static_cast<uint8_t>((id * 7 + 31) % 256);
        for (int y = 0; y < BLOCK_H; ++y) {
            for (int x = 0; x < BLOCK_W; ++x) {
                bool border = (x == 0 || y == 0 || x == 15 || y == 15);
                bool pattern = ((x + y + id) % 4 == 0);
                ts.pixels[id * BLOCK_PIXELS + y * BLOCK_W + x] = border ? c2 : (pattern ? c1 : static_cast<uint8_t>((c1 + 40) % 256));
            }
        }
    }
}

// رسم بلوك واحد 16x16 مع التكبير (scale)
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
