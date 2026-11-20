#pragma once
#include <cstddef>
#include <utility>
#include <functional>
#include <initializer_list>
#include <iterator>

namespace STDLib {
    template<typename Key, typename Value>
    struct MapNode {
        using RetValue = std::pair<Key, Value>;

        MapNode(const Key& k, const Value& v) : kvpair{k, v} {}
        MapNode(Key&& k, Value&& v) : kvpair{std::move(k), std::move(v)} {}


        RetValue kvpair;

        MapNode* next = nullptr;
    };

    template<typename Key, typename Value>
    class MapIterator {
        using MapNodeType = MapNode<Key, Value>;

    public:
        //-----------STL compatibility header-----------
        using iterator_category = std::forward_iterator_tag;
        using value_type = typename MapNodeType::RetValue;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;
        //-----------STL compatibility header-----------

        MapIterator() : node{nullptr} {}
        MapIterator(MapNodeType* node) : node{node} {}

        MapIterator& operator++() {
            node = node->next;

            return *this;
        }

        MapIterator operator++(int) {
            MapNodeType* copy = node;
            node = node->next;

            return copy;
        }

        reference operator*() {
            return node->kvpair;
        }

        pointer operator->() {
            return &node->kvpair;
        }

        pointer base() {
            return &node->kvpair;
        }

        pointer base() const {
            return &node->kvpair;
        }

        MapNodeType* baseNode() {
            return node;
        }

        const MapNodeType* baseNode() const {
            return node;
        }

        bool operator==(const MapIterator& obj) {
            return node == obj.baseNode();
        }

        bool operator!=(const MapIterator& obj) {
            return !(*this == obj);
        }

    private:
        MapNodeType* node;
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

        Value& at(const Key& key);
        Value& at(const Key& key) const;

        size_t size() const;

        Value* find(const Key& key) const;

        void insert(const Key& key, const Value& value);

        void erase(const Key& key);
        void erase(Iterator pos);

        Value& operator[](const Key& key);
        // Value& operator[](const Key& key) const;

        void clear();

        Iterator begin();
        Iterator end();

    private:
        MapNode<Key, Value>* head = nullptr;
        size_t count = 0;
        Compare cmp;
    };
}

#include <libstd/impl/map_impl.thi>