#include <memory>
#include <optional>
#include <utility>
#include <cstddef>
#include <libkbase/string.hpp> // strlen()
#include <libkbase/memory.hpp> // memset(), memcpy(), memmove(), memcmp()
#include <libkrt/krt.hpp>
#include <libkstd/string.hpp>

namespace KernelSTD {
    String::String(const char* str) { insert(0, str); }
    String::String(const char* str, size_t size) { insert(0, str, size); }
    String::String(char ch, size_t size) { insert(0, ch, size); }
    String::String(std::string_view view) { insert(begin(), view.begin(), view.end()); }
    String::String(const String& obj) { copy(obj); }
    String::String(String&& obj) { swap(obj); }

    String& String::operator=(const char* str) {
        clear();

        insert(0, str);

        return *this;
    }

    String& String::operator=(std::string_view view) {
        clear();

        insert(begin(), view.begin(), view.end());

        return *this;
    }

    String& String::operator=(const String& obj) {
        copy(obj);

        return *this;
    }

    String& String::operator=(String&& obj) {
        swap(obj);

        return *this;
    }

    String& String::operator+=(char ch) {
        append(ch);

        return *this;
    }

    String& String::operator+=(const char* str) {
        append(str);

        return *this;
    }

    String& String::operator+=(std::string_view view) {
        insert(end(), view.begin(), view.end());

        return *this;
    }

    String& String::operator+=(const String& obj) {
        append(obj);

        return *this;
    }

    char& String::operator[](size_t index) {
        return buffer[index];
    }

    void String::copy(const String& obj) {
        clear();

        insert(0, obj.buffer.get(), obj.length);
    }

    void String::swap(String& obj) {
        std::swap(capacity, obj.capacity);
        std::swap(length, obj.length);
        std::swap(buffer, obj.buffer);
    }

    void String::resize(size_t size){
        std::unique_ptr<char[]> newbuffer{new char[size]};

        memcpy(newbuffer.get(), buffer.get(), std::min(size, length));

        capacity = size;
        buffer = std::move(newbuffer);
    }

    size_t String::size() const {
        return length;
    }

    char& String::at(size_t index) {
        if (index >= length) KernelRT::abort();

        return buffer[index];
    }

    void String::append(char ch) {
        if (length >= capacity) resize(length + 1);

        buffer[length++] = ch;
    }

    void String::append(char ch, size_t size) {
        if (length + size > capacity) resize(length + size);

        while (size--) buffer[length++] = ch;
    }

    void String::append(const char* str) {
        append(str, strlen(str));
    }

    void String::append(const char* str, size_t size) {
        if (length + size > capacity) resize(length + size);

        while (size--) buffer[length++] = *(str++);
    }

    void String::append(const String& obj) {
        append(obj.buffer.get(), obj.length);
    }

    void String::insert(size_t pos, char ch) {
        insert(pos, ch, 1);
    }

    void String::insert(size_t pos, char ch, size_t size) {
        if (length + size > capacity) resize(length + size);

        // for (size_t index = length - 1; index >= pos; index--) buffer[index + size] = buffer[index];
        memmove(buffer.get() + pos + size, buffer.get() + pos, length - pos);
        memset(buffer.get() + pos, ch, size);

        length += size;
    }

    void String::insert(size_t pos, const char* str) {
        insert(pos, str, strlen(str));
    }

    void String::insert(size_t pos, const char* str, size_t size) {
        if (length + size > capacity) resize(length + size);

        // for (size_t index = length - 1; index >= pos; index--) buffer[index + size] = buffer[index];
        memmove(buffer.get() + pos + size, buffer.get() + pos, length - pos);
        memcpy(buffer.get() + pos, str, size);

        length += size;
    }

    void String::insert(size_t pos, const String& obj) {
        insert(pos, obj.buffer.get(), obj.length);
    }

    void String::insert(Iterator pos, char ch) {
        insert(pos - begin(), ch);
    }

    void String::insert(Iterator pos, char ch, size_t size) {
        insert(pos - begin(), ch, size);
    }

    void String::insert(Iterator pos, const char* str) {
        insert(pos - begin(), str);
    }

    void String::insert(Iterator pos, const char* str, size_t size) {
        insert(pos - begin(), str, size);
    }

    void String::insert(Iterator pos, const String& obj) {
        insert(pos - begin(), obj);
    }

    void String::erase(size_t pos) {
        erase(pos, 1);
    }

    void String::erase(size_t pos, size_t size) {
        memmove(buffer.get() + pos, buffer.get() + pos + size, length - (pos + size));

        length -= size;
    }

    void String::erase(Iterator pos) {
        erase(pos - begin());
    }

    void String::erase(Iterator begin, Iterator end) {
        erase(begin - this->begin(), end - begin);
    }

    void String::push_back(char ch) {
        append(ch);
    }

    void String::pop_back() {
        length--;
    }

    char* String::data() {
        return buffer.get();
    }

    const char* String::data() const {
        return buffer.get();
    }

    void String::clear() {
        length = 0;
    }

    String String::substr(size_t pos, size_t size) {
        return String{begin() + pos, begin() + pos + size};
    }

    std::optional<size_t> String::find(char ch) {
        for (auto it = begin(); it != end(); it++) if (*it == ch) return it - begin();

        return std::nullopt;
    }

    std::optional<size_t> String::find(const char* str) {
        size_t windowSize = std::min(length, strlen(str));

        for (auto it = begin(); it != end(); it++) if (!memcmp(it.base(), str, windowSize)) return it - begin();

        return std::nullopt;
    }

    std::optional<size_t> String::find(const String& obj) {
        for (auto it = begin(); it != end(); it++) if (!memcmp(it.base(), obj.buffer.get(), obj.length)) return it - begin();

        return std::nullopt;
    }

    String::Iterator String::begin() {
        return buffer.get();
    }

    String::Iterator String::end() {
        return buffer.get() + length;
    }

    String::ConstIterator String::begin() const {
        return buffer.get();
    }

    String::ConstIterator String::end() const {
        return buffer.get() + length;
    }

    String::ReverseIterator String::rbegin() {
        return ReverseIterator{end()};
    }

    String::ReverseIterator String::rend() {
        return ReverseIterator{begin()};
    }

    String::ConstReverseIterator String::rbegin() const {
        return ConstReverseIterator{end()};
    }

    String::ConstReverseIterator String::rend() const {
        return ConstReverseIterator{begin()};
    }

    std::string_view String::view() {
        return std::string_view{buffer.get(), length};
    }

    String::operator std::string_view() {
        return view();
    }

    String operator+(const String& lhs, const String& rhs) {
        String tmp;
        tmp += lhs;
        tmp += rhs;

        return tmp;
    }

    String operator+(const String& lhs, char rhs) {
        String tmp;
        tmp += lhs;
        tmp += rhs;

        return tmp;
    }

    String operator+(char lhs, const String& rhs) {
        String tmp;
        tmp += lhs;
        tmp += rhs;

        return tmp;
    }

    bool operator==(const String& lhs, const String& rhs) {
        if (lhs.size() != rhs.size()) return false;

        return !memcmp(lhs.data(), rhs.data(), lhs.size());
    }

    bool operator==(const String& lhs, char rhs) {
        if (lhs.size() == 1);

        return *lhs.begin() == rhs;
    }

    bool operator==(char lhs, const String& rhs) {
        if (rhs.size() == 1);

        return *rhs.begin() == lhs;
    }

    bool operator!=(const String& lhs, const String& rhs) {
        return !(lhs == rhs);
    }

    bool operator!=(const String& lhs, char rhs) {
        return !(lhs == rhs);
    }

    bool operator!=(char lhs, const String& rhs) {
        return !(lhs == rhs);
    }
}