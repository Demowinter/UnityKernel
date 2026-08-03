#pragma once
#include <cstddef>
#include <cstdint>
#include <utility>
#include <type_traits>
#include <memory>

namespace KernelSTD {

template<typename T>
struct DefaultHash {
    std::size_t operator()(const T& v) const noexcept {
        if constexpr (std::is_integral<T>::value) {
            return static_cast<std::size_t>(v);
        } else {
            // Fallback: use pointer representation
            return reinterpret_cast<std::size_t>(std::addressof(v));
        }
    }
};

template<typename T>
struct DefaultEqual { bool operator()(const T& a, const T& b) const noexcept { return a == b; } };

template<typename Key, typename Value, std::size_t BucketCount = 64, typename Hash = DefaultHash<Key>, typename Equal = DefaultEqual<Key>>
class HashMap {
public:
    struct Node { Key key; Value value; Node* next{nullptr}; Node(const Key& k, const Value& v): key(k), value(v) {} };

    HashMap() {
        buckets = new Node*[BucketCount];
        for (std::size_t i = 0; i < BucketCount; ++i) buckets[i] = nullptr;
    }

    ~HashMap() { clear(); delete[] buckets; }

    void clear() {
        for (std::size_t i = 0; i < BucketCount; ++i) {
            Node* cur = buckets[i];
            while (cur) { Node* n = cur->next; delete cur; cur = n; }
            buckets[i] = nullptr;
        }
        count = 0;
    }

    std::size_t size() const { return count; }

    Value* find(const Key& key) {
        std::size_t h = hasher(key) % BucketCount;
        Node* cur = buckets[h];
        while (cur) { if (equal(cur->key, key)) return &cur->value; cur = cur->next; }
        return nullptr;
    }

    void insert(const Key& key, const Value& value) {
        std::size_t h = hasher(key) % BucketCount;
        Node* cur = buckets[h];
        while (cur) { if (equal(cur->key, key)) { cur->value = value; return; } cur = cur->next; }
        Node* n = new Node(key, value);
        n->next = buckets[h]; buckets[h] = n; ++count;
    }

    void erase(const Key& key) {
        std::size_t h = hasher(key) % BucketCount;
        Node* cur = buckets[h]; Node* prev = nullptr;
        while (cur) {
            if (equal(cur->key, key)) {
                if (prev) prev->next = cur->next; else buckets[h] = cur->next;
                delete cur; --count; return;
            }
            prev = cur; cur = cur->next;
        }
    }

    Value& operator[](const Key& key) {
        Value* v = find(key);
        if (v) return *v;
        insert(key, Value());
        return *find(key);
    }

private:
    Node** buckets{nullptr};
    std::size_t count{0};
    Hash hasher;
    Equal equal;
};

} // namespace KernelSTD
