#pragma once
#include <cstddef>
#include <utility>
#include <functional>

namespace KernelSTD {

template<typename Key, typename Value, typename Compare>
class MapBase {
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
};

template<typename Key, typename Value, typename Compare = std::less<Key>>
class Map : protected MapBase<Key, Value, Compare> {
public:
    using Pair = std::pair<Key, Value>;
    Map() = default;
    ~Map() { clear(); }

    using typename MapBase<Key, Value, Compare>::Entry;

    void clear() { this->clear_impl(); }

    std::size_t size() const { return this->count; }

    Value* find(const Key& key) {
        auto cur = this->head;
        while (cur) {
            if (!this->cmp(cur->key, key) && !this->cmp(key, cur->key)) return &cur->value;
            cur = cur->next;
        }
        return nullptr;
    }

    void insert(const Key& key, const Value& value) {
        Entry* prev = nullptr;
        Entry* cur = this->head;
        while (cur && this->cmp(cur->key, key)) { prev = cur; cur = cur->next; }
        if (cur && !this->cmp(cur->key, key) && !this->cmp(key, cur->key)) { cur->value = value; return; }
        Entry* e = new Entry(key, value);
        if (!prev) { e->next = this->head; this->head = e; }
        else { e->next = prev->next; prev->next = e; }
        ++this->count;
    }

    void erase(const Key& key) {
        Entry* prev = nullptr;
        Entry* cur = this->head;
        while (cur && (this->cmp(cur->key, key) || this->cmp(key, cur->key))) { prev = cur; cur = cur->next; }
        if (!cur) return;
        if (prev) prev->next = cur->next; else this->head = cur->next;
        delete cur; --this->count;
    }

    Value& operator[](const Key& key) {
        Value* v = find(key);
        if (v) return *v;
        insert(key, Value());
        return *find(key);
    }
};

} // namespace KernelSTD
