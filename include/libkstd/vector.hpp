#pragma once
#include <memory>
#include <initializer_list>
#include <algorithm>
#include <iterator>
#include <cstddef>
#include <libkstd/iterator.hpp>

namespace KernelSTD {
    template<typename Type>
    class Vector {
    public:
        using Iterator = NormalIterator<Type*>;
        using ReverseIterator = std::reverse_iterator<Iterator>;

        Vector() = default;
        Vector(size_t size);
        Vector(const Type& data, size_t size);
        Vector(std::initializer_list<Type> list);
        Vector(const Vector& obj);
        Vector(Vector&& obj);

        template<typename InputIterator>
        Vector(InputIterator begin, InputIterator end) { insert(this->begin(), begin, end); }

        Vector& operator=(std::initializer_list<Type> list);
        Vector& operator=(const Vector& obj);
        Vector& operator=(Vector&& obj);

        void copy(const Vector& obj);
        void swap(Vector& obj);

        void resize(size_t size);

        size_t capacity() const;
        size_t size() const;

        Type& operator[](size_t index);
        Type& at(size_t index);

        void insert(size_t pos, const Type& value);
        void insert(size_t pos, const Type& value, size_t size);
        void insert(Iterator pos, const Type& value);
        void insert(Iterator pos, const Type& value, size_t size);

        template<typename InputIterator>
        void insert(Iterator pos, InputIterator begin, InputIterator end) {
            size_t posNum = pos - this->begin();

            for (auto it = begin; it != end; it++) insert(posNum++, *it);
        }

        void erase(size_t pos);
        void erase(size_t pos, size_t size);
        void erase(Iterator pos);
        void erase(Iterator begin, Iterator end);

        void push_back(const Type& value);
        void pop_back();

        char* data();

        void clear();

        Iterator begin();
        Iterator end();

        ReverseIterator rbegin();
        ReverseIterator rend();
    private:
        std::unique_ptr<Type[]> buffer = nullptr;

        size_t buffCapacity = 0;
        size_t buffSize = 0;
    };
}

#include <libkstd/impl/vector_impl.thi>