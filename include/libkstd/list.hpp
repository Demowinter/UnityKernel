#pragma once
#include <memory>
#include <initializer_list>
#include <iterator>
#include <cstddef>

namespace KernelSTD {
    template<typename Type>
    struct LinkedNode {
        Type value;

        LinkedNode* next = nullptr;
        LinkedNode* prev = nullptr;
    };

    template<typename Type>
    class ListIterator {
    public:
        //-----------STL compatibility header-----------
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = Type;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        //-----------STL compatibility header-----------

        ListIterator() : node{nullptr} {}
        ListIterator(LinkedNode<Type>* node) : node{node} {}

        ListIterator& operator++() {
            node = node->next;

            return *this;
        }

        ListIterator& operator--() {
            node = node->prev;

            return *this;
        }

        ListIterator operator++(int) {
            LinkedNode<Type>* copy = node;
            node = node->next;

            return copy;
        }

        ListIterator operator--(int) {
            LinkedNode<Type>* copy = node;
            node = node->prev;

            return copy;
        }

        reference operator*() {
            return node->value;    
        }

        pointer operator->() {
            return &node->value;
        }

        pointer base() {
            return &node->value;
        }

        const pointer base() const {
            return &node->value;
        }

        LinkedNode<Type>* baseNode() {
            return node;
        }

        const LinkedNode<Type>* baseNode() const {
            return node;
        }

        bool operator==(const ListIterator& obj) {
            return node == obj.baseNode();
        }

        bool operator!=(const ListIterator& obj) {
            return !(*this == obj);
        }

    private:
        LinkedNode<Type>* node;
    };

    template<typename Type>
    class LinkedList {
    public:
        using Iterator = ListIterator<Type>;
        using ReverseIterator = std::reverse_iterator<Iterator>;

        LinkedList() = default;
        LinkedList(const Type& data, size_t size);
        LinkedList(std::initializer_list<Type> list);
        LinkedList(const LinkedList& obj);
        LinkedList(LinkedList&& obj);
        ~LinkedList();

        template<typename InputIterator>
        LinkedList(InputIterator begin, InputIterator end) { insert(this->begin(), begin, end); }

        LinkedList& operator=(std::initializer_list<Type> list);
        LinkedList& operator=(const LinkedList& obj);
        LinkedList& operator=(LinkedList&& obj);

        void copy(const LinkedList& obj);
        void swap(LinkedList& obj);

        size_t size() const;

        void insert(Iterator pos, const Type& value);
        void insert(Iterator pos, const Type& value, size_t size);

        template<typename InputIterator>
        void insert(Iterator pos, InputIterator begin, InputIterator end) {
            for (auto it = begin; it != end; it++) insert(pos, *it);
        }

        void erase(Iterator pos);
        void erase(Iterator begin, Iterator end);

        void push_back(const Type& value);
        void push_front(const Type& value);

        void pop_back();
        void pop_front();

        void clear();

        Iterator begin();
        Iterator end();

        ReverseIterator rbegin();
        ReverseIterator rend();

    private:
        LinkedNode<Type>* head = new LinkedNode<Type>;
        LinkedNode<Type>* tail = head;
        size_t count = 0;
    };
}

#include <libkstd/impl/list_impl.thi>