#include "stupid_trie.h"
#include <iostream>


int main()
{
    stupid_trie<int> stupid;
    stupid.emplace("gsd",42);
    stupid.emplace("gsd",43);
    stupid.emplace("gs", 69);
    std::cout << "fasz" << std::endl;
}