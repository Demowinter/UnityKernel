#pragma once
#include <memory>
#include <string_view>
#include <optional>
#include <iterator>
#include <cstddef>
#include <libkstd/iterator.hpp>

namespace KernelSTD {
    class String {
    public:
        using Iterator = NormalIterator<char>;
        using ReverseIterator = std::reverse_iterator<Iterator>;

        String() = default;
        String(const char* str);
        String(const char* str, size_t size);
        String(char ch, size_t size);
        String(const String& obj);
        String(String&& obj);

        template<class InputIterator>
        String(InputIterator begin, InputIterator end) { insert(this->begin(), begin, end); }

        String& operator=(const char* str);
        String& operator=(const String& obj);
        String& operator=(String&& obj);

        String& operator+=(char ch);
        String& operator+=(const char* str);
        String& operator+=(const String& obj);

        char& operator[](size_t index);

        void copy(const String& obj);
        void swap(String& obj);

        void resize(size_t size);
        size_t size();

        std::optional<std::reference_wrapper<char>> at(size_t index);

        void append(char ch);
        void append(char ch, size_t size);
        void append(const char* str);
        void append(const char* str, size_t size);
        void append(const String& obj);

        void insert(size_t pos, char ch);
        void insert(size_t pos, char ch, size_t size);
        void insert(size_t pos, const char* str);
        void insert(size_t pos, const char* str, size_t size);
        void insert(size_t pos, const String& obj);
        void insert(Iterator pos, char ch);
        void insert(Iterator pos, char ch, size_t size);
        void insert(Iterator pos, const char* str);
        void insert(Iterator pos, const char* str, size_t size);
        void insert(Iterator pos, const String& obj);

        template<class InputIterator>
        void insert(Iterator pos, InputIterator begin, InputIterator end) {
            insert(pos, begin.base(), end - begin);
        }

        void erase(size_t pos);
        void erase(size_t pos, size_t size);
        void erase(Iterator pos);
        void erase(Iterator begin, Iterator end);

        void push_back(char ch);
        void pop_back();

        char* data();
        const char* c_str() const; 

        void clear();

        String substr(size_t pos, size_t size);

        std::optional<size_t> find(char ch);
        std::optional<size_t> find(const char* str);
        std::optional<size_t> find(const String& obj);

        Iterator begin();
        Iterator end();

        ReverseIterator rbegin();
        ReverseIterator rend();

        std::string_view view();

        operator std::string_view();

    private:
        std::unique_ptr<char[]> buffer;

        size_t capacity = 0;
        size_t length = 0;
    };
}