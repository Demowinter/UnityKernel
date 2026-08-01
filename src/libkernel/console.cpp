#include <string_view>
#include <cstddef>
#include <cstdint>
#include <libkernel/console.hpp>

namespace Kernel::Console {
    namespace {
        constexpr uint32_t glyphWidth = 5;
        constexpr uint32_t glyphHeight = 7;
        constexpr uint32_t scale = 1;
        constexpr uint32_t cellWidth = (glyphWidth + 1) * scale;
        constexpr uint32_t cellHeight = (glyphHeight + 1) * scale;
        constexpr uint8_t defaultColor = 0x0F;

        Framebuffer framebuffer{};
        uint32_t cursorX = 0;
        uint32_t cursorY = 0;

        struct Glyph {
            uint8_t rows[glyphHeight];
        };

        constexpr Glyph blank = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

        const Glyph& glyphFor(char character) {
            static constexpr Glyph digits[] = {
                {{0x0E, 0x11, 0x13, 0x15, 0x19, 0x11, 0x0E}},
                {{0x04, 0x0C, 0x04, 0x04, 0x04, 0x04, 0x0E}},
                {{0x0E, 0x11, 0x01, 0x02, 0x04, 0x08, 0x1F}},
                {{0x1E, 0x01, 0x01, 0x0E, 0x01, 0x01, 0x1E}},
                {{0x02, 0x06, 0x0A, 0x12, 0x1F, 0x02, 0x02}},
                {{0x1F, 0x10, 0x1E, 0x01, 0x01, 0x11, 0x0E}},
                {{0x06, 0x08, 0x10, 0x1E, 0x11, 0x11, 0x0E}},
                {{0x1F, 0x01, 0x02, 0x04, 0x08, 0x08, 0x08}},
                {{0x0E, 0x11, 0x11, 0x0E, 0x11, 0x11, 0x0E}},
                {{0x0E, 0x11, 0x11, 0x0F, 0x01, 0x02, 0x0C}},
            };
            static constexpr Glyph letters[] = {
                {{0x0E, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}}, // A
                {{0x1E, 0x11, 0x11, 0x1E, 0x11, 0x11, 0x1E}}, // B
                {{0x0E, 0x11, 0x10, 0x10, 0x10, 0x11, 0x0E}}, // C
                {{0x1E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x1E}}, // D
                {{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x1F}}, // E
                {{0x1F, 0x10, 0x10, 0x1E, 0x10, 0x10, 0x10}}, // F
                {{0x0E, 0x11, 0x10, 0x17, 0x11, 0x11, 0x0E}}, // G
                {{0x11, 0x11, 0x11, 0x1F, 0x11, 0x11, 0x11}}, // H
                {{0x0E, 0x04, 0x04, 0x04, 0x04, 0x04, 0x0E}}, // I
                {{0x01, 0x01, 0x01, 0x01, 0x11, 0x11, 0x0E}}, // J
                {{0x11, 0x12, 0x14, 0x18, 0x14, 0x12, 0x11}}, // K
                {{0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x1F}}, // L
                {{0x11, 0x1B, 0x15, 0x15, 0x11, 0x11, 0x11}}, // M
                {{0x11, 0x19, 0x15, 0x13, 0x11, 0x11, 0x11}}, // N
                {{0x0E, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}}, // O
                {{0x1E, 0x11, 0x11, 0x1E, 0x10, 0x10, 0x10}}, // P
                {{0x0E, 0x11, 0x11, 0x11, 0x15, 0x12, 0x0D}}, // Q
                {{0x1E, 0x11, 0x11, 0x1E, 0x14, 0x12, 0x11}}, // R
                {{0x0F, 0x10, 0x10, 0x0E, 0x01, 0x01, 0x1E}}, // S
                {{0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04}}, // T
                {{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x0E}}, // U
                {{0x11, 0x11, 0x11, 0x11, 0x11, 0x0A, 0x04}}, // V
                {{0x11, 0x11, 0x11, 0x15, 0x15, 0x15, 0x0A}}, // W
                {{0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11}}, // X
                {{0x11, 0x11, 0x0A, 0x04, 0x04, 0x04, 0x04}}, // Y
                {{0x1F, 0x01, 0x02, 0x04, 0x08, 0x10, 0x1F}}, // Z
            };
            static constexpr Glyph colon = {{0x00, 0x04, 0x04, 0x00, 0x04, 0x04, 0x00}};
            static constexpr Glyph dot = {{0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C}};
            static constexpr Glyph dash = {{0x00, 0x00, 0x00, 0x1F, 0x00, 0x00, 0x00}};
            static constexpr Glyph bang = {{0x04, 0x04, 0x04, 0x04, 0x04, 0x00, 0x04}};

            if (character >= 'a' && character <= 'z') {
                character -= 'a' - 'A';
            }
            if (character >= 'A' && character <= 'Z') return letters[character - 'A'];
            if (character >= '0' && character <= '9') return digits[character - '0'];
            if (character == ':') return colon;
            if (character == '.') return dot;
            if (character == '-') return dash;
            if (character == '!') return bang;
            return blank;
        }

        uint32_t color(uint8_t value) {
            static constexpr uint32_t palette[] = {
                0x000000, 0x0000AA, 0x00AA00, 0x00AAAA,
                0xAA0000, 0xAA00AA, 0xAA5500, 0xAAAAAA,
                0x555555, 0x5555FF, 0x55FF55, 0x55FFFF,
                0xFF5555, 0xFF55FF, 0xFFFF55, 0xFFFFFF,
            };
            const uint32_t rgb = palette[value & 0x0F];
            const uint8_t red = rgb >> 16;
            const uint8_t green = rgb >> 8;
            const uint8_t blue = rgb;

            return (uint32_t(red >> (8 - framebuffer.redMaskSize)) << framebuffer.redPosition) |
                   (uint32_t(green >> (8 - framebuffer.greenMaskSize)) << framebuffer.greenPosition) |
                   (uint32_t(blue >> (8 - framebuffer.blueMaskSize)) << framebuffer.bluePosition);
        }

        void putPixel(uint32_t x, uint32_t y, uint32_t pixel) {
            if (x >= framebuffer.width || y >= framebuffer.height) return;
            auto* row = reinterpret_cast<volatile uint32_t*>(framebuffer.address + y * framebuffer.pitch);
            row[x] = pixel;
        }

        void eraseRow(uint32_t row) {
            const uint32_t background = color(0x00);
            for (uint32_t y = row * cellHeight; y < (row + 1) * cellHeight && y < framebuffer.height; ++y) {
                for (uint32_t x = 0; x < framebuffer.width; ++x) putPixel(x, y, background);
            }
        }

        void scroll() {
            auto* pixels = reinterpret_cast<volatile uint32_t*>(framebuffer.address);
            const uint32_t stride = framebuffer.pitch / sizeof(uint32_t);
            const uint32_t movedRows = framebuffer.height > cellHeight ? framebuffer.height - cellHeight : 0;

            for (uint32_t y = 0; y < movedRows; ++y) {
                for (uint32_t x = 0; x < framebuffer.width; ++x) {
                    pixels[y * stride + x] = pixels[(y + cellHeight) * stride + x];
                }
            }
            eraseRow((framebuffer.height / cellHeight) - 1);
        }

        void drawGlyph(char character, uint8_t foreground, uint8_t background) {
            const Glyph& glyph = glyphFor(character);
            const uint32_t originX = cursorX * cellWidth;
            const uint32_t originY = cursorY * cellHeight;
            const uint32_t foregroundPixel = color(foreground);
            const uint32_t backgroundPixel = color(background);

            for (uint32_t y = 0; y < cellHeight; ++y) {
                for (uint32_t x = 0; x < cellWidth; ++x) {
                    const bool set = x < glyphWidth * scale && y < glyphHeight * scale &&
                        (glyph.rows[y / scale] & (1u << (4 - x / scale))) != 0;
                    putPixel(originX + x, originY + y, set ? foregroundPixel : backgroundPixel);
                }
            }
        }

        void advanceLine() {
            cursorX = 0;
            ++cursorY;
            if (cursorY >= framebuffer.height / cellHeight) {
                scroll();
                cursorY = framebuffer.height / cellHeight - 1;
            }
        }

        void putChar(char character, uint8_t foreground) {
            if (character == '\n') {
                advanceLine();
                return;
            }
            if (character == '\r') {
                cursorX = 0;
                return;
            }
            if (character == '\t') {
                const uint32_t spaces = 4 - (cursorX % 4);
                for (uint32_t i = 0; i < spaces; ++i) putChar(' ', foreground);
                return;
            }

            drawGlyph(character, foreground, 0x00);
            if (++cursorX >= framebuffer.width / cellWidth) advanceLine();
        }
    }

    bool initialize(const Framebuffer& newFramebuffer) {
        if (newFramebuffer.address == 0 || newFramebuffer.bpp != 32 || newFramebuffer.type != 1 ||
            newFramebuffer.width < cellWidth || newFramebuffer.height < cellHeight ||
            newFramebuffer.pitch < newFramebuffer.width * sizeof(uint32_t)) {
            return false;
        }

        framebuffer = newFramebuffer;
        cursorX = 0;
        cursorY = 0;
        return true;
    }

    bool isReady() {
        return framebuffer.address != 0;
    }

    void clear() {
        if (!isReady()) return;
        for (uint32_t y = 0; y < framebuffer.height; ++y) {
            for (uint32_t x = 0; x < framebuffer.width; ++x) putPixel(x, y, color(0x00));
        }
        cursorX = 0;
        cursorY = 0;
    }

    void newline() {
        if (isReady()) advanceLine();
    }

    void write(std::byte byte) {
        if (isReady()) putChar(static_cast<char>(byte), defaultColor);
    }

    void write(const void* str, uint32_t size, uint8_t color) {
        if (!isReady()) return;
        for (uint32_t i = 0; i < size; ++i) putChar(reinterpret_cast<const char*>(str)[i], color);
    }

    void print(std::string_view str, uint8_t color) {
        if (!isReady()) return;
        for (char ch : str) putChar(ch, color);
    }

    void println(std::string_view str, uint8_t color) {
        print(str, color);
        newline();
    }
}
