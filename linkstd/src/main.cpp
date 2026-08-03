#include <iostream>
#include <string>
#include "linkstd/linked_list.hpp"
#include "linkstd/map.hpp"
#include "linkstd/hashmap.hpp"

int main() {
    using namespace LinkSTD;

    LinkedList<int> list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);

    std::cout << "LinkedList contents:";
    for (auto it = list.begin(); it != list.end(); ++it) std::cout << ' ' << *it;
    std::cout << '\n';

    Map<std::string,int> map;
    map.insert("one", 1);
    map.insert("two", 2);
    map.insert("three", 3);
    std::cout << "Map[two] = " << map["two"] << '\n';

    HashMap<int,std::string> hmap;
    hmap.insert(10, "ten");
    hmap.insert(20, "twenty");
    auto* v = hmap.find(10);
    std::cout << "HashMap[10] = " << (v ? *v : std::string("(null)")) << '\n';

    return 0;
}
