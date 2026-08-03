#pragma once
#include <cstddef>
#include <utility>

namespace KernelSTD {

template<typename T>
class LinkedList {
public:
    struct Node {
        T value;
        Node* prev{nullptr};
        Node* next{nullptr};
        Node(const T& v): value(v) {}
        Node(T&& v): value(std::move(v)) {}
    };

    LinkedList() = default;
    ~LinkedList() { clear(); }

    LinkedList(const LinkedList&) = delete;
    LinkedList& operator=(const LinkedList&) = delete;

    void push_back(const T& v) {
        Node* n = new Node(v);
        if (!tail) head = tail = n;
        else { tail->next = n; n->prev = tail; tail = n; }
        ++count;
    }

    void push_back(T&& v) {
        Node* n = new Node(std::move(v));
        if (!tail) head = tail = n;
        else { tail->next = n; n->prev = tail; tail = n; }
        ++count;
    }

    void push_front(const T& v) {
        Node* n = new Node(v);
        if (!head) head = tail = n;
        else { head->prev = n; n->next = head; head = n; }
        ++count;
    }

    bool empty() const { return count == 0; }
    std::size_t size() const { return count; }

    void clear() {
        Node* cur = head;
        while (cur) {
            Node* next = cur->next;
            delete cur;
            cur = next;
        }
        head = tail = nullptr;
        count = 0;
    }

    Node* front_node() { return head; }
    Node* back_node() { return tail; }

    void remove(Node* node) {
        if (!node) return;
        if (node->prev) node->prev->next = node->next;
        else head = node->next;
        if (node->next) node->next->prev = node->prev;
        else tail = node->prev;
        delete node;
        --count;
    }

    struct Iterator {
        Node* ptr{nullptr};
        Iterator() = default;
        explicit Iterator(Node* p): ptr(p) {}
        T& operator*() const { return ptr->value; }
        T* operator->() const { return &ptr->value; }
        Iterator& operator++() { ptr = ptr->next; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ptr = ptr->next; return tmp; }
        Iterator& operator--() { ptr = ptr->prev; return *this; }
        bool operator==(const Iterator& o) const { return ptr == o.ptr; }
        bool operator!=(const Iterator& o) const { return ptr != o.ptr; }
    };

    Iterator begin() { return Iterator(head); }
    Iterator end() { return Iterator(nullptr); }

private:
    Node* head{nullptr};
    Node* tail{nullptr};
    std::size_t count{0};
};

} // namespace KernelSTD
