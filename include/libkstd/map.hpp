#pragma once
#include <cstddef>
#include <utility>
#include <functional>
#include <initializer_list>
#include <iterator>

namespace KernelSTD {
    template<typename Key, typename Value>
    struct MapNode {
        Key key;
        Value value;

        MapNode* next = nullptr;

        MapNode(const Key& k, const Value& v) : key(k), value(v) {}
        MapNode(Key&& k, Value&& v) : key(std::move(k)), value(std::move(v)) {}
    };

    template<typename Key, typename Value>
    class MapIterator {
    public:
        //-----------STL compatibility header-----------
        using iterator_category = std::forward_iterator_tag;
        using value_type = Value;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        //-----------STL compatibility header-----------

        MapIterator() : node{nullptr} {}
        MapIterator(MapNode<Key, Value>* node) : node{node} {}

        MapIterator& operator++() {
            node = node->next;

            return *this;
        }

        MapIterator operator++(int) {
            MapNode<Key, Value>* copy = node;
            node = node->next;

            return copy;
        }

        reference operator*() {
            return node->value;
        }

        pointer operator->() {
            return &node->value;
        }

        const Key& key() const {
            return node->key;
        }

        pointer base() {
            return &node->value;
        }

        const pointer base() const {
            return &node->value;
        }

        MapNode<Key, Value>* baseNode() {
            return node;
        }

        const MapNode<Key, Value>* baseNode() const {
            return node;
        }

        bool operator==(const MapIterator& obj) {
            return node == obj.baseNode();
        }

        bool operator!=(const MapIterator& obj) {
            return !(*this == obj);
        }

    private:
        MapNode<Key, Value>* node;
    };

    template<typename Key, typename Value, typename Compare = std::less<Key>>
    class Map {
    public:
        using Iterator = MapIterator<Key, Value>;

        Map() = default;
        Map(std::initializer_list<std::pair<const Key, Value>> list);
        Map(const Map& obj);
        Map(Map&& obj);
        ~Map();

        Map& operator=(std::initializer_list<std::pair<const Key, Value>> list);
        Map& operator=(const Map& obj);
        Map& operator=(Map&& obj);

        void copy(const Map& obj);
        void swap(Map& obj);

        size_t size() const;

        Value* find(const Key& key);

        void insert(const Key& key, const Value& value);
        void erase(const Key& key);

        Value& operator[](const Key& key);

        void clear();

        Iterator begin();
        Iterator end();

    private:
        MapNode<Key, Value>* head = nullptr;
        size_t count = 0;
        Compare cmp;
    };
}

#include <libkstd/impl/map_impl.thi>