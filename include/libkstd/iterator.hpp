#pragma once
#include <iterator>

namespace KernelSTD {
    template<typename Type>
    class NormalIterator {
    public:
        //-----------STL compatibility header-----------
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Type;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        //-----------STL compatibility header-----------

        NormalIterator() : ptr{nullptr} {}
        NormalIterator(pointer ptr) : ptr{ptr} {}

        NormalIterator& operator++() {
            ptr++;

            return *this;
        }

        NormalIterator& operator--() {
            ptr--;

            return *this;
        }

        NormalIterator operator++(int) {
            pointer copy = ptr;
            ptr++;

            return copy;
        }

        NormalIterator operator--(int) {
            pointer copy = ptr;
            ptr++;

            return copy;
        }

        NormalIterator& operator+=(difference_type diff) {
            ptr += diff;

            return *this;
        }

        NormalIterator& operator-=(difference_type diff) {
            ptr -= diff;

            return *this;
        }

        NormalIterator operator+(difference_type diff) {
            return ptr + diff;
        }

        NormalIterator operator-(difference_type diff) {
            return ptr - diff;
        }

        reference operator[](difference_type diff) {
            return ptr[diff];
        }

        reference operator*() {
            return *ptr;    
        }

        pointer operator->() {
            return ptr;
        }

        pointer base() {
            return ptr;
        }

        const pointer base() const {
            return ptr;
        }

        difference_type operator-(const NormalIterator& it) const {
            return ptr - it.base();
        }
    private:
        pointer ptr;
    };

    template<typename Type>
    class ConstIterator {
    public:
        //-----------STL compatibility header-----------
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Type;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        //-----------STL compatibility header-----------

        ConstIterator() : ptr{nullptr} {}
        ConstIterator(pointer ptr) : ptr{ptr} {}
        ConstIterator(const NormalIterator<Type>& it) : ptr{it.base()} {}

        ConstIterator operator+(difference_type diff) const {
            return ptr + diff;
        }

        ConstIterator operator-(difference_type diff) const {
            return ptr - diff;
        }

        const reference operator*() const;
        const pointer operator->() const;

        const pointer base() const;

        difference_type operator-(const ConstIterator& it) const;

        bool operator==(const ConstIterator& it) const;
        bool operator!=(const ConstIterator& it) const;

    private:
        pointer ptr;
    };

    template<typename Type>
    bool operator==(const NormalIterator<Type>& it1, const NormalIterator<Type>& it2) {
        return it1.base() == it2.base();
    }

    template<typename Type>
    bool operator!=(const NormalIterator<Type>& it1, const NormalIterator<Type>& it2) {
        return !(it1 == it2);
    }

    template<typename Type>
    bool operator<(const NormalIterator<Type>& it1, const NormalIterator<Type>& it2) {
        return it1.base() < it2.base();
    }

    template<typename Type>
    bool operator<=(const NormalIterator<Type>& it1, const NormalIterator<Type>& it2) {
        return it1.base() <= it2.base();
    }

    template<typename Type>
    bool operator>(const NormalIterator<Type>& it1, const NormalIterator<Type>& it2) {
        return it1.base() > it2.base();
    }

    template<typename Type>
    bool operator>=(const NormalIterator<Type>& it1, const NormalIterator<Type>& it2) {
        return it1.base() >= it2.base();
    }
}