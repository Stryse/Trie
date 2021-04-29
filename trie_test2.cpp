#include "stupid_trie.h"
#include <iostream>
#include <cassert>

int main()
{
    // stupid_trie<int> stupid;
    // auto InsertGSD = stupid.emplace("gsd",42);
    
    // assert(InsertGSD.first->first == "gsd" && InsertGSD.first->second == 42 && InsertGSD.second == true);


    // auto insertGSD2 = stupid.emplace("gsd",43);
    // auto insertGS = stupid.emplace("gs", 69);
    // std::cout << stupid.count("gsd") << std::endl;
    // std::cout << stupid.count("fasz") << std::endl;
    // auto insertFasz = stupid.emplace("fasz",420);
    // std::cout << stupid.count("fasz") << std::endl;

    stupid_trie<int> STI;
  static_assert(std::is_same_v<decltype(STI)::key_type, std::string>);
  static_assert(std::is_same_v<decltype(STI)::mapped_type, int>);
  static_assert(std::is_same_v<decltype(STI)::value_type,
                               std::pair<const std::string, int>>);

  assert(STI.empty() && STI.size() == 0 && STI.count("whispy") == 0);
  //STI.count(static_cast<void*>(0)); // !!! Should not compile.

  const decltype(STI)& cSTI = STI;
  // Callable on const.
  assert(STI.empty() && cSTI.size() == 0 && cSTI.count("whispy") == 0);

  // Like std::map emplace.
  auto InsertGSD = STI.emplace("gsd", 42);
  auto InsertWhispy = STI.emplace("whispy", 69);
  auto InsertXazax = STI.emplace("xazax", 1337);

  assert(!cSTI.empty() && cSTI.size() == 3);
  assert(cSTI.count("gsd") == 1 && cSTI.count("whispy") == 1 &&
         cSTI.count("xazax") == 1);

  assert(InsertGSD.first->first == "gsd" && InsertGSD.first->second == 42 &&
         InsertGSD.second == true);
  assert(InsertWhispy.first->first == "whispy" &&
         InsertWhispy.first->second == 69 && InsertWhispy.second == true);
  assert(InsertXazax.first->first == "xazax" &&
         InsertXazax.first->second == 1337 && InsertXazax.second == true);

  auto InsertGSDAgain = STI.emplace("gsd", 43);
  // Insertion does not happen, gsd is already inserted, return iterator to
  // already existing element.
  assert(InsertGSDAgain.second == false && InsertGSDAgain.first->second == 42);

  //cSTI.emplace("inserting into const should not happen", -1); // !!! Should not compile.
  printf("asd");
}