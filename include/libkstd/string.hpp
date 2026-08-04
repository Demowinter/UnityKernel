#pragma once
#include <memory>
#include <cstddef>
#include <libkstd/iterator.hpp>
#include <libkbase/string.hpp>

namespace KernelSTD {
    class String {
        class ForwardIterator : public Iterator<char, ForwardIterator> {
        public:
            ForwardIterator(char* ptr) : ptr(ptr) {}

            void increment(size_t size) { ptr += size; }
            void decrement(size_t size) { ptr -= size; }

            char* base() { return ptr; }
            const char* base() const { return ptr; }

        private:
            char* ptr;
        };

        class ReverseIterator : public Iterator<char, ReverseIterator> {
        public:
            ReverseIterator(char* ptr, bool sentiel = false);

            void increment(size_t size) { ptr -= size; }
            void decrement(size_t size) { ptr += size; }

            char* base() { return ptr; }
            const char* base() const { return ptr; }

        private:
            char* ptr;
        };

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

        ForwardIterator begin() {
            return buffer.get();
        }

    private:
        std::unique_ptr<char[]> buffer;

        size_t capacity;
        size_t size;
    };
}