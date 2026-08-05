#pragma once
#include <memory>
#include <cstddef>

namespace KernelSTD {
    class String {
        class ForwardIterator;
        class ReverseIterator;

    public:
        String() = default;
        String(const char* str);
        String(const char* str, size_t size);
        String(char ch, size_t size);
        String(const String& obj);
        String(String&& obj);

        String& operator=(const char*);
        String& operator=(const String& obj);
        String& operator=(String&& obj);

        String& operator+=(const char*);
        String& operator+=(const String& obj);

        char& operator[](size_t index);

        void copy(const String&);
        void swap(String&);

        void resize();

        ForwardIterator begin();
        ForwardIterator end();

    private:
        std::unique_ptr<char[]> buffer;

        size_t capacity;
        size_t size;
    };

    class String::ForwardIterator {
    public:
        ForwardIterator(char* ptr);

        ForwardIterator& operator++();
        ForwardIterator& operator--();

        ForwardIterator operator++(int);
        ForwardIterator operator--(int);

        ForwardIterator& operator+=(size_t size);
        ForwardIterator& operator-=(size_t size);

        ForwardIterator operator+(size_t size);
        ForwardIterator operator-(size_t size);

        char& operator*();
        char* operator->();

        char* base();
        const char* base() const;

    private:
        char* ptr = nullptr;
    };

    class String::ReverseIterator {
    public:
        ReverseIterator(char* ptr);

        ReverseIterator& operator++();
        ReverseIterator& operator--();

        ReverseIterator operator++(int);
        ReverseIterator operator--(int);

        ReverseIterator& operator+=(size_t size);
        ReverseIterator& operator-=(size_t size);

        ReverseIterator operator+(size_t size);
        ReverseIterator operator-(size_t size);

        char& operator*();
        char* operator->();

        char* base();
        const char* base() const;

    private:
        char* ptr = nullptr;
    };
}