#pragma once
#include <cstddef>

namespace KernelSTD {
    template<class T, class Derived>
    class Iterator {
    public:
        Derived& operator++() {
            toDerived().increment(1);

            return toDerived();
        }

        Derived& operator--() {
            toDerived().decrement(1);

            return toDerived();
        }

        Derived operator++(int) {
            Derived tmp = toDerived();

            ++(*this);
            // operator++();

            return tmp;
        }

        Derived operator--(int) {
            Derived tmp = toDerived();

            --(*this);
            // operator--();

            return tmp;
        }

        Derived& operator+=(size_t size) {
            toDerived().increment(size);

            return toDerived();
        }

        Derived& operator-=(size_t size) {
            toDerived().decrement(size);

            return toDerived();
        }

        Derived operator+(size_t size)  {
            Derived tmp = toDerived();
            tmp.increment(size);

            return tmp;
        }

        Derived operator-(size_t size)  {
            Derived tmp = toDerived();
            tmp.decrement(size);

            return tmp;
        }

        T& operator*() {
            return *base();
        }

        T* operator->() {
            return base();
        }

        T* base() { return toDerived().base(); }
        const T* base() const { return toDerived().base(); }
    private:
        Derived& toDerived() { return static_cast<Derived&>(*this); }
    };

    template<class T, class Derived>
    inline bool operator==(const Iterator<T, Derived>& it1, const Iterator<T, Derived>& it2) {
        return it1.base() == it2.base();
    }
}