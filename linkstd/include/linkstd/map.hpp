#pragma once
#include <cstddef>
#include <utility>
#include <functional>

namespace LinkSTD {

template<typename Key, typename Value, typename Compare = std::less<Key>>
class Map {
protected:
    struct Entry {
        Key key;
        Value value;
        Entry* next{nullptr};
        Entry(const Key& k, const Value& v): key(k), value(v) {}
        Entry(Key&& k, Value&& v): key(std::move(k)), value(std::move(v)) {}
    };

    Entry* head{nullptr};
    std::size_t count{0};
    Compare cmp;

    void clear_impl() {
        Entry* cur = head;
        while (cur) { Entry* n = cur->next; delete cur; cur = n; }
        head = nullptr; count = 0;
    }

public:
    Map() = default;
    ~Map() { clear(); }

    void clear() { clear_impl(); }

    std::size_t size() const { return count; }

    Value* find(const Key& key) {
        auto cur = head;
        while (cur) {
            if (!cmp(cur->key, key) && !cmp(key, cur->key)) return &cur->value;
            cur = cur->next;
        }
        return nullptr;
    }

    void insert(const Key& key, const Value& value) {
        Entry* prev = nullptr;
        Entry* cur = head;
        while (cur && cmp(cur->key, key)) { prev = cur; cur = cur->next; }
        if (cur && !cmp(cur->key, key) && !cmp(key, cur->key)) { cur->value = value; return; }
        Entry* e = new Entry(key, value);
        if (!prev) { e->next = head; head = e; }
        else { e->next = prev->next; prev->next = e; }
        ++count;
    }

    void erase(const Key& key) {
        Entry* prev = nullptr;
        Entry* cur = head;
        while (cur && (cmp(cur->key, key) || cmp(key, cur->key))) { prev = cur; cur = cur->next; }
        if (!cur) return;
        if (prev) prev->next = cur->next; else head = cur->next;
        delete cur; --count;
    }

    Value& operator[](const Key& key) {
        Value* v = find(key);
        if (v) return *v;
        insert(key, Value());
        return *find(key);
    }
};

} // namespace LinkSTD
