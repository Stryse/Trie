#ifndef STUPID_TRIE__H
#define STUPID_TRIE__H

#include <string>
#include <utility>
#include <optional>
#include <vector>
#include <algorithm>

template<typename _Tp, 
         typename _Compare = std::less<std::string>>

class stupid_trie
{

protected:
    class trie_node;

public:
    /********************************* Member types *********************************/

    using key_type        = std::string;
    using mapped_type     = _Tp;
    using value_type      = std::pair<const std::string, _Tp>;
    using size_type       = size_t;
    using key_compare     = _Compare;
    using reference       = value_type&;
    using const_reference = const value_type&;
    // iterator
    // const_iterator
    // reverse_iterator
    // const_reverse_iterator
    using node_type       = trie_node;
    // insert_return_type
    /*********************************************************************************/

protected:
    /******************************** Member classes ********************************/

    struct trie_node
    {
        key_type key;
        std::optional<mapped_type> value;

        std::vector<trie_node> children;

        trie_node() 
            : key{}
        {}

        trie_node(const key_type& key)
            : key(key)
        {}

        trie_node(key_type&& key)
            : key(std::move(key))
        {}

        trie_node(const value_type& kv_pair)
            : key(kv_pair.first), value(kv_pair.second)
        {}

        trie_node(value_type&& kv_pair)
            : key(std::move(kv_pair.first)),value(std::move(kv_pair.second))
        {}

        trie_node(const key_type& key, const mapped_type& value)
            : key(key), value(value)
        {}

        trie_node(key_type&& key, mapped_type&& value)
            : key(std::move(key)), value(std::move(value))
        {}

        virtual ~trie_node()                    = default;
        trie_node(const trie_node&)             = default;
        trie_node(trie_node&&)                  = default;
        trie_node& operator=(const trie_node&)  = default;
        trie_node& operator=(trie_node&&)       = default;
    };

    struct node_compare
    {
        key_compare compare;

        node_compare(const key_compare& compare)
            : compare(compare){}

        bool operator()(const node_type& lhs, const node_type& rhs)
        {
            return compare(lhs.key,rhs.key);
        }
    };

public:
    /********************************* Constructors **********************************/
    explicit stupid_trie(const _Compare& compare = _Compare{}) 
        : _size{}, _compare{compare}
    {}

    stupid_trie(const stupid_trie&) = default;
    stupid_trie(stupid_trie&&)      = default;
    virtual ~stupid_trie()          = default;
    /*********************************************************************************/

    bool   empty() const noexcept { return _size == 0;  }
    size_t size()  const noexcept { return _size;       } 




    void emplace(key_type&& key, mapped_type&& value)
    {
        key_type local_key(std::move(key));
        node_type* current_node = &_root;

        for(size_t i = 1; i <= local_key.size(); ++i)
        {
            key_type keyPiece = local_key.substr(0,i);

            auto branch = std::find_if(current_node->children.begin(), current_node->children.end(), 
                                       [&](const node_type& child) { return child.key == keyPiece; });

            if(branch == current_node->children.end())
            {
                auto& next_node = current_node->children.emplace_back(std::move(keyPiece));

                std::sort(current_node->children.begin(),
                          current_node->children.end(),
                          _compare);

                current_node = &next_node;
            }
            else
                current_node = std::addressof(*branch);
        }

        // TODO: return iterator
        if(!current_node->value.has_value())
        {
            current_node->value.emplace(std::move(value));
            ++_size;
        }
        else
        {
        }
    }


private:

    size_t _size;    
    node_type _root;
    node_compare _compare;
};

#endif /* STUPID_TRIE__H */