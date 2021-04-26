#include "stupid_trie.h"
#include <iostream>


int main()
{
    stupid_trie<int> stupid;
    stupid.emplace("gsd",42);
    stupid.emplace("gsd",43);
    stupid.emplace("gs", 69);
    std::cout << stupid.count("gsd") << std::endl;
    std::cout << stupid.count("fasz") << std::endl;
    stupid.emplace("fasz",420);
    std::cout << stupid.count("fasz") << std::endl;
}