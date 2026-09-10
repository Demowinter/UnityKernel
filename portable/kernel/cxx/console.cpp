#include <string_view>
#include <cstddef>
#include <cstdint>
#include <kernel/console.hpp>

namespace Kernel::Console {
    static uint16_t* video = reinterpret_cast<uint16_t*>(0xB8000);
    static uint16_t cursor = 0;

    //Input buffer (circular, 256 characters)
    static constexpr size_t inputBufferSize = 256;
    static char inputBuffer[inputBufferSize];
    static size_t inputHead = 0;   // next write position
    static size_t inputTail = 0;   // next read position
    static size_t inputCount = 0;  // number of characters in buffer

    void scroll() {
        for (size_t i = columns; i < screenSize; i++) video[i - columns] = video[i];
        for (size_t i = screenSize - columns; i < screenSize; i++) video[i] = 0x00;

        cursor -= columns;
    }

    void checkScroll() {
        if (cursor >= screenSize)
            while (cursor >= screenSize) scroll();
    }

    void clear() {
        for (size_t i = 0; i < screenSize; i++) video[i] = 0x00;

        cursor = 0;
    }

    void newline() {
        checkScroll();

        cursor += columns - (cursor % columns);

        checkScroll();
    }

    void write(std::byte byte, uint8_t color) {
        checkScroll();

        video[cursor++] = (color << 8) | static_cast<uint8_t>(byte);
    }

    void write(const void* str, size_t size, uint8_t color) {
        for (size_t i = 0; i < size; i++) write(std::byte{reinterpret_cast<const uint8_t*>(str)[i]}, color);
    }

    void print(std::string_view str, uint8_t color) {
        for (char ch : str) write(std::byte{static_cast<uint8_t>(ch)}, color);
    }

    void println(std::string_view str, uint8_t color) {
        print(str, color);

        newline();
    }

    void ok(std::string_view str, bool newline) {
        print("[  ");
        print("OK", 0x0A);
        print("  ] ");

        print(str);

        if (newline) Console::newline();
    }

    void info(std::string_view str, bool newline) {
        print("[ ");
        print("INFO", 0x0F);
        print(" ] ");

        print(str);

        if (newline) Console::newline();
    }

    void warn(std::string_view str, bool newline) {
        print("[ ");
        print("WARN", 0x0E);
        print(" ] ");

        print(str);

        if (newline) Console::newline();
    }

    void fail(std::string_view str, bool newline) {
        print("[");
        print("FAILED", 0x0C);
        print("] ");

        print(str);

        if (newline) Console::newline();
    }

    //============ Input functions ============

    //Scancode to ASCII translation table (US layout, no shift)
    static constexpr char scancodeToAscii[] = {
        0,   27,  '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', '\b', '\t',
        'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n', 0,   'a', 's',
        'd', 'f', 'g', 'h', 'j', 'k', 'l', ';', '\'', '`', 0,   '\\', 'z', 'x', 'c', 'v',
        'b', 'n', 'm', ',', '.', '/', 0,   '*', 0,   ' ',  0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0
    };

    //Scancode to ASCII translation table (US layout, with shift)
    static constexpr char scancodeToAsciiShift[] = {
        0,   27,  '!', '@', '#', '$', '%', '^', '&', '*', '(', ')', '_', '+', '\b', '\t',
        'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\n', 0,   'A', 'S',
        'D', 'F', 'G', 'H', 'J', 'K', 'L', ':', '"', '~', 0,   '|', 'Z', 'X', 'C', 'V',
        'B', 'N', 'M', '<', '>', '?', 0,   '*', 0,   ' ',  0,   0,   0,   0,   0,   0,
        0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   '-', 0,   0,   0,   '+', 0
    };

    void onKeyPress(uint8_t scancode, bool shift, bool ctrl, bool alt) {
        //Ignore key releases (scancode >= 128)
        if (scancode >= 128) return;
        if (scancode >= sizeof(scancodeToAscii)) return;

        char ascii;
        if (shift) {
            ascii = scancodeToAsciiShift[scancode];
        } else {
            ascii = scancodeToAscii[scancode];
        }

        if (ascii == 0) return;  // Ignore non-printable keys

        //Handle special keys
        if (ctrl) {
            if (ascii == 'c' || ascii == 'C') {
                ascii = 3;  // Ctrl+C
            }
        }

        //Add to input buffer if not full
        if (inputCount < inputBufferSize) {
            inputBuffer[inputHead] = ascii;
            inputHead = (inputHead + 1) % inputBufferSize;
            inputCount++;
        }
    }

    void putchar(char ch, uint8_t color) {
        if (ch == '\n') {
            newline();
        } else if (ch == '\b') {
            //Backspace - move cursor back and erase
            if (cursor > 0) {
                cursor--;
                write(std::byte{' '}, color);
                cursor--;
            }
        } else {
            write(std::byte{static_cast<uint8_t>(ch)}, color);
        }
    }

    char read() {
        //Blocking read
        while (inputCount == 0) {
            asm volatile("hlt");  //Wait for interrupt
        }

        char ch = inputBuffer[inputTail];
        inputTail = (inputTail + 1) % inputBufferSize;
        inputCount--;

        return ch;
    }

    bool tryRead(char& ch) {
        //Non-blocking read
        if (inputCount == 0) return false;

        ch = inputBuffer[inputTail];
        inputTail = (inputTail + 1) % inputBufferSize;
        inputCount--;

        return true;
    }

    size_t readline(char* buffer, size_t size) {
        size_t pos = 0;
        char ch;

        while (pos < size - 1) {
            ch = read();

            if (ch == '\n') {
                buffer[pos] = '\0';
                putchar('\n');
                return pos;
            } else if (ch == '\b') {
                if (pos > 0) {
                    pos--;
                    putchar('\b');
                }
            } else if (ch >= 32 && ch < 127) {
                buffer[pos++] = ch;
                putchar(ch);
            }
        }

        buffer[pos] = '\0';
        return pos;
    }
}