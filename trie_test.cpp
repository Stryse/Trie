#include <cassert>
#include <functional>
#include <memory>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "stupid_trie.h"
#include "generic_trie.h"

/** http://enwp.org/Trie
 *  --------------------

In computer science, a trie, also called digital tree or prefix tree, is a type
of search tree, a tree data structure used for locating specific keys from
within a set. These keys are most often strings, with links between nodes
defined not by the entire key, but by individual characters. In order to access
a key (to recover its value, change it, or remove it), the trie is traversed
depth-first, following the links between nodes, which represent each character
in the key.

*/

// Extra challenge: implement your own optional!
template <typename T>
using optional = std::optional<T>;

int stupid() {
  // Conventional trie: map strings to some T.
  // Try implement as much as the std::map interface as possible. Most of it
  // applies to our use case.

  // The internal representation of the nodes should be something like this:
  /*

g
├─ gs
│  ├─ gsd -> 42
w
├─ wh
│  ├─ whi
│  │  ├─ whis
│  │  │  ├─ whisp
│  │  │  │  ├─ whispy -> 69
x
├─ xa
│  ├─ xaz
│  │  ├─ xaza
│  │  │  ├─ xazax -> 1337

  */

  stupid_trie<int> STI;
  static_assert(std::is_same_v<decltype(STI)::key_type, std::string>);
  static_assert(std::is_same_v<decltype(STI)::mapped_type, int>);
  static_assert(std::is_same_v<decltype(STI)::value_type,
                               std::pair<const std::string, int&>>); // Átírtam a value_type-t pair<const K, V&>-re

  assert(STI.empty() && STI.size() == 0 && STI.count("whispy") == 0);
  // STI.count(static_cast<void*>(0)); // !!! Should not compile.

  const decltype(STI)& cSTI = STI;
  // Callable on const.
  assert(STI.empty() && cSTI.size() == 0 && cSTI.count("whispy") == 0);

  // Like std::map emplace. ( Átrendeztem az iterátor asserteket, mert az emplace-m invalidálja az előző iterátorokat mivel érték szerint tárol a vectorban)
  auto InsertGSD = STI.emplace("gsd", 42);
  assert(InsertGSD.first->first == "gsd" && InsertGSD.first->second == 42 &&
         InsertGSD.second == true);

  auto InsertWhispy = STI.emplace("whispy", 69);
  assert(InsertWhispy.first->first == "whispy" &&
         InsertWhispy.first->second == 69 && InsertWhispy.second == true);

  auto InsertXazax = STI.emplace("xazax", 1337);
  assert(InsertXazax.first->first == "xazax" &&
         InsertXazax.first->second == 1337 && InsertXazax.second == true);

  assert(!cSTI.empty() && cSTI.size() == 3);
  assert(cSTI.count("gsd") == 1 && cSTI.count("whispy") == 1 &&
         cSTI.count("xazax") == 1);


  auto InsertGSDAgain = STI.emplace("gsd", 43);
  // Insertion does not happen, gsd is already inserted, return iterator to
  // already existing element.
  assert(InsertGSDAgain.second == false && InsertGSDAgain.first->second == 42);

  // cSTI.emplace("inserting into const should not happen", -1); // !!! Should not compile.

  try {
    STI.at("foo");
    assert(false && "Should have been unreachable.");
  } catch (const std::out_of_range&) {
  }

  try {
    cSTI.at("bar");
    assert(false && "Should have been unreachable.");
  } catch (const std::out_of_range&) {
  }

  // This is where we diverge from the std::map interface a little bit.
  // operator[] will not return a default constructed T like it does for map,
  // but instead an optional!
  auto MaybeElement = STI["gsd"];
  static_assert(std::is_same_v<decltype(MaybeElement), std::optional<std::reference_wrapper<int>>>); // átírtam ref_wrapper<T> -re T-ről

  // And because we return optional, operator[] is viable on const instances!
  const auto MaybeElementOnConst = cSTI["abel"];
  static_assert(std::is_same_v<decltype(MaybeElementOnConst),
                               const std::optional<std::reference_wrapper<const int>>>); // átírtam ref_wrapper<T> -re T-ről

  assert(MaybeElement.has_value() && MaybeElement.value() == 42);
  assert(!MaybeElementOnConst.has_value()); // Kitöröltem a value_or-t mivel ref wrapperbe nem lehet átadni sima számot

  try {
    STI.emplace("This Element Does Not Exist", MaybeElementOnConst.value());
    assert(false && "Should have been unreachable.");
  } catch (const std::bad_optional_access&) {
  }

  assert(cSTI.count("This Element Does Not Exist") == 0);

  // The elements should be iterated in the natural order of the keys, in this
  // case, lexicographical.
  std::ostringstream OS;
  for (const decltype(STI)::value_type& Elem : STI) {
    OS << '(' << Elem.first << "->" << Elem.second << "),";
  }
  std::string Result = OS.str();
  Result.pop_back();
  std::string Expected = "(gsd->42),(whispy->69),(xazax->1337)";
  assert(Result == Expected);

  STI["gsd"].value().get() = 43; // Átírtam .value().get()-re
  STI.emplace("abel", 16);

  // Now we add a new value. CAREFUL: "gs" is prefix of "gsd"! This should
  // definitely not break our internal representation!
  STI.emplace("gs", -24);

  OS.str("");
  for (const decltype(STI)::value_type& Elem : STI) {
    OS << '(' << Elem.first << "->" << Elem.second << "),";
  }
  Result = OS.str();
  Result.pop_back();
  Expected = "(abel->16),(gs->-24),(gsd->43),(whispy->69),(xazax->1337)";
  assert(Result == Expected);

  return 1;
}

int stupid_noncopyable() {
  stupid_trie<int> CopyableTrie;
  CopyableTrie.emplace("foo", 1);
  CopyableTrie.emplace("bar", 2);
  std::ostringstream OS;
  for (const decltype(CopyableTrie)::value_type& Elem : CopyableTrie) {
    OS << '(' << Elem.first << "->" << Elem.second << "),";
  }
  std::string Result = OS.str();
  Result.pop_back();
  std::string Expected = "(bar->2),(foo->1)";
  assert(Result == Expected);

  decltype(CopyableTrie) Copy = CopyableTrie; // Int can be copied.
  Copy["foo"].value().get() = 8; // Átírtam .value().get()-re
  OS.str("");
  for (const decltype(CopyableTrie)::value_type& Elem : CopyableTrie) {
    OS << '(' << Elem.first << "->" << Elem.second << "),";
  }
  Result = OS.str();
  Result.pop_back();
  Expected = "(bar->2),(foo->1)"; // Changing Copy doesn't change original obj.
  assert(Result == Expected);

  stupid_trie<std::unique_ptr<int>> NonCopyableTrie;
  NonCopyableTrie.emplace("int1", std::make_unique<int>(1234));

  // // decltype(NonCopyableTrie) Copy2 = NonCopyableTrie;
  // // !!! Should not compile, unique_ptrs, and thus the nodes of it are not copyable!

  decltype(NonCopyableTrie) Moved = std::move(NonCopyableTrie); // Should work.

  OS.str("");
  for (const decltype(Moved)::value_type& Elem : Moved) {
    OS << '(' << Elem.first << "->" << *Elem.second.get() << "),";
  }
  Result = OS.str();
  Result.pop_back();
  Expected = "(int1->1234)"; // Changing Copy doesn't change original obj.
  assert(Result == Expected);

  return 1;
}

int generic() {
  // Alright, let's go all in this time. The problem with the conventional trie
  // is that std::strings might be expensive to store. There is also no need to
  // store "g" and "gs" and "gsd", there should only be 'g', 's', and 'd' stored
  // in the nodes to get to the value of "gsd". This is what a **real** trie
  // does.
  //
  // To make this work, we will need a few type parameters.
  //  - The individual key piece type. In our case, it's char.
  //  - The mapped type. Could be anything, let's do int.
  //  - We will need to be able to concatenate the key pieces together.
  //  - We will need to be able to compare the key pieces.
  //  - We need a type that represents the **full** key to the user. This is
  //    string.
  //  - Do not forget, string types usually take two additional template
  //    parameters, the "char traits" and the allocator. We need to pass these
  //    too.
  //
  // The interface shall take ***TEMPLATE TEMPLATES*** for the last three type
  // parameters! (This is so that you look into what "template templates" are.)
  //
  // Other predicatesd and functors might also be needed, feel free to
  // experiment!
  const auto& CharToStringConcat = [](std::string& Seq, char C)
      -> std::string& {
    Seq.push_back(C);
    return Seq;
  };

  using default_template_parameters = trie<char, int,
                                           decltype(CharToStringConcat)>;
  static_assert(std::is_same_v<default_template_parameters::key_type,
                               std::basic_string<char>>);
  static_assert(std::is_same_v<default_template_parameters::value_type,
                               std::pair<const std::basic_string<char>, int&>>); // Átírtam a valut_type-t pair<const K,T&>-re
  static_assert(std::is_same_v<default_template_parameters::key_compare,
                               std::less<char>>);
  static_assert(std::is_same_v<default_template_parameters::key_concat,
                               decltype(CharToStringConcat)>);
  static_assert(std::is_same_v<default_template_parameters::key_type
                                 ::traits_type,
                               std::char_traits<char>>);
  static_assert(std::is_same_v<default_template_parameters::key_type
                                 ::allocator_type,
                               std::allocator<char>>);

  using fully_specified = trie<char,
                               int,
                               decltype(CharToStringConcat),
                               std::less,
                               std::basic_string,
                               std::char_traits,
                               std::allocator>;
  static_assert(std::is_same_v<default_template_parameters, fully_specified>);


  // The internal representation of the nodes should be something like this:
  /*

g
├─ s
│  ├─ d -> 42
w
├─ h
│  ├─ i
│  │  ├─ s
│  │  │  ├─ p
│  │  │  │  ├─ y -> 69
x
├─ a
│  ├─ z
│  │  ├─ a
│  │  │  ├─ x -> 1337

  */

  // The interface for the generic trie shall be roughly the same as the stupid
  // one's. It is only the representation that is different, emphasising cache
  // locality and smaller memory footprint.
  default_template_parameters GTI{CharToStringConcat};

  assert(GTI.empty() && GTI.size() == 0 && GTI.count("whispy") == 0);
  //GTI.count(static_cast<void*>(0)); // !!! Should not compile.

  const decltype(GTI)& cGTI = GTI;
  // Callable on const.
  assert(GTI.empty() && cGTI.size() == 0 && cGTI.count("whispy") == 0);

  // // Like std::map emplace. // Az assertek-et kicsit átrendeztem mert az emplace-m invalidálja az iterátorokat
                               // mert érték szerint tárolom a gyerekeket vektorban
  auto InsertGSD = GTI.emplace("gsd", 42);
  assert(InsertGSD.first->first == "gsd" && InsertGSD.first->second == 42 &&
         InsertGSD.second == true);

  auto InsertWhispy = GTI.emplace("whispy", 69);
  assert(InsertWhispy.first->first == "whispy" &&
         InsertWhispy.first->second == 69 && InsertWhispy.second == true);

  auto InsertXazax = GTI.emplace("xazax", 1337);
  assert(InsertXazax.first->first == "xazax" &&
         InsertXazax.first->second == 1337 && InsertXazax.second == true);

  assert(!cGTI.empty() && cGTI.size() == 3);
  assert(cGTI.count("gsd") == 1 && cGTI.count("whispy") == 1
         && cGTI.count("xazax") == 1);


  auto InsertGSDAgain = GTI.emplace("gsd", 43);
  // Insertion does not happen, gsd is already inserted, return iterator to
  // already existing element.
  assert(InsertGSDAgain.second == false && InsertGSDAgain.first->second == 42);

  //cGTI.emplace("inserting into const should not happen", -1); // !!! Should not compile.

  try {
    GTI.at("foo");
    assert(false && "Should have been unreachable.");
  } catch (const std::out_of_range&) {
  }

  try {
    cGTI.at("bar");
    assert(false && "Should have been unreachable.");
  } catch (const std::out_of_range&) {
  }

  // This is where we diverge from the std::map interface a little bit.
  // operator[] will not return a default constructed T like it does for map,
  // but instead an optional!
  auto MaybeElement = GTI["gsd"];
  static_assert(std::is_same_v<decltype(MaybeElement), optional<std::reference_wrapper<int>>>); // átírtam ref wrapperre

  // // And because we return optional, operator[] is viable on const instances!
  const auto MaybeElementOnConst = cGTI["abel"];
  static_assert(std::is_same_v<decltype(MaybeElementOnConst),
                               const optional<std::reference_wrapper<const int>>>); // átírtam ref wrapperre

  assert(MaybeElement.has_value() && MaybeElement.value() == 42);
  assert(!MaybeElementOnConst.has_value()); // value_ort kitöröltem ref wrapper miatt
  try {
    GTI.emplace("This Element Does Not Exist", MaybeElementOnConst.value());
    assert(false && "Should have been unreachable.");
  } catch (const std::bad_optional_access&) {
  }

  assert(cGTI.count("This Element Does Not Exist") == 0);

  // The elements should be iterated in the natural order of the keys, in this
  // case, lexicographical.
  std::ostringstream OS;
  for (const decltype(GTI)::value_type& Elem : GTI) {
    OS << '(' << Elem.first << "->" << Elem.second << "),";
  }
  std::string Result = OS.str();
  Result.pop_back();
  std::string Expected = "(gsd->42),(whispy->69),(xazax->1337)";
  assert(Result == Expected);

  GTI["gsd"].value().get() = 43; // ref wrapper miatt value get
  GTI.emplace("abel", 16);

  // Now we add a new value. CAREFUL: "gs" is prefix of "gsd"! This should
  // definitely not break our internal representation!
  GTI.emplace("gs", -24);
  // Be really really careful here: you'll have to ensure that the node for
  // "gs" (node g -> node s) exists properly, and place the value inside there.
  // Use optional<T> for your advantage!

  OS.str("");
  for (const decltype(GTI)::value_type& Elem : GTI) {
    OS << '(' << Elem.first << "->" << Elem.second << "),";
  }
  Result = OS.str();
  Result.pop_back();
  Expected = "(abel->16),(gs->-24),(gsd->43),(whispy->69),(xazax->1337)";
  assert(Result == Expected);

  // Reverse iterator test
  OS.str("");
  for (auto it = GTI.rbegin(); it != GTI.rend(); ++it)
  {
    OS << '(' << it.base()->first << "->" << it.base()->second << "),";
  }
  Result = OS.str();
  Result.pop_back();
  Expected = "(xazax->1337),(whispy->69),(gsd->43),(gs->-24),(abel->16)";
  assert(Result == Expected);

  // Find test
  auto findWhispy = GTI.find("whispy");
  assert(findWhispy != GTI.end() && findWhispy->first == "whispy" && findWhispy->second == 69);

  auto findGregorics = GTI.find("Gregorics");
  assert(findGregorics == GTI.end() && "No Grego Gang");

  // Erase test
  GTI.erase("gs");
  assert(GTI.count("gs") == 0 && GTI.count("gsd") == 1);

  auto GT = GTI.emplace("Gregorics", 420).first;
  assert(GTI.count("Gregorics") == 1);

  GTI.erase(GT);
  assert(GTI.count("Gregorics") == 0);

  return 1;
}

/** Additional excercise
 *  --------------------

Think about Huffmann encoding and Huffmann trees. Huffmann trees store the
encoding key to the individual symbols based on the order of descent in the
binary tree. Going left is a '0', going right is a '1', and the tree is binary.

But the fact that getting to the key is based on the descent itself, it means
Huffmann trees are tries too!
However, storing the key as *string* would be wasteful: you could store bytes
only, and the full key is some sufficiently large number (unsigned long long,
or __uint128_t or larger if your compiler supports it). The "concatenation"
is shifting the bit into the key under work.

Use the above trie template (preferably **WITHOUT** an explicit specialisation!)
with <unsigned long long, bool, ...> to work as a Huffmann tree!

*/

int main() {
  int8_t grade = 1;
  if (stupid() && stupid_noncopyable())
    ++grade;
  if (generic())
    ++grade;
  return grade;
}