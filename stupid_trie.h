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
    using node_type       = trie_node;
    /*********************************************************************************/

protected:
    /******************************** Member classes ********************************/
    struct trie_node
    {
        key_type first;
        std::optional<mapped_type> second;

        std::vector<trie_node> children;

        trie_node() 
            : first{}
        {}

        trie_node(const key_type& key)
            : first(key)
        {}

        trie_node(key_type&& key)
            : first(std::move(key))
        {}

        trie_node(const value_type& kv_pair)
            : first(kv_pair.first), second(kv_pair.second)
        {}

        trie_node(value_type&& kv_pair)
            : first(std::move(kv_pair.first)), second(std::move(kv_pair.second))
        {}

        trie_node(const key_type& key, const mapped_type& value)
            : first(key), second(value)
        {}

        trie_node(key_type&& key, mapped_type&& value)
            : first(std::move(key)), second(std::move(value))
        {}

        virtual ~trie_node()                    = default;
        trie_node(const trie_node&)             = default;
        trie_node(trie_node&&)                  = default;
        trie_node& operator=(const trie_node&)  = default;
        trie_node& operator=(trie_node&&)       = default;
    };

    /********************************************************
     * @brief Compares nodes with provided template argument
     * key_compare in order to maintain class invariance in
     * children vectors.
     ********************************************************/
    struct node_compare
    {
        node_compare(const key_compare& compare)
            : compare(compare){}

        bool operator()(const node_type& lhs, const node_type& rhs)
        {
            return compare(lhs.first,rhs.first);
        }

        key_compare compare;
    };

public:
    class iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = node_type;
        using pointer           = node_type*;
        using reference         = node_type&;

        iterator(node_type* ptr) : _pointed_node(ptr) {}
        ~iterator()               = default;
        iterator(const iterator&) = default;
        iterator(iterator&&)      = default;

        iterator& operator=(const iterator&) = default;
        iterator& operator=(iterator&&)      = default;

        reference operator* () const { return *_pointed_node; }
        pointer   operator->() const { return  _pointed_node; }
        
        //iterator& operator++()
        //{

        //}

        //iterator operator++(int)
        //{

        //}

        friend bool operator==(const iterator& lhs, const iterator& rhs) { return lhs._pointed_node == rhs._pointed_node; }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) { return !(lhs == rhs); }

    private:
        node_type* _pointed_node;
    };

    iterator begin() const noexcept { return iterator(&_root);  }
    iterator end()   const noexcept { return iterator(nullptr); }

public:
    /********************************* Constructors **********************************/
    explicit stupid_trie(const _Compare& compare = _Compare{}) 
        : _size{}, _rightmost{&_root},_compare{compare}
    {}

    stupid_trie(const stupid_trie&) = default;
    stupid_trie(stupid_trie&&)      = default;
    virtual ~stupid_trie()          = default;
    /*********************************************************************************/

    bool   empty() const noexcept { return _size == 0;  }
    size_t size()  const noexcept { return _size;       } 


    // TODO: return iterator
    std::pair<iterator,bool> emplace(key_type&& key, mapped_type&& value)
    {
        key_type local_key(std::move(key));
        node_type* current_node = &_root;

        for(size_t i = 1; i <= local_key.size(); ++i)
        {
            key_type keyPiece = local_key.substr(0,i);

            auto branch = std::find_if(current_node->children.begin(), current_node->children.end(), 
                                       [&](const node_type& child) { return child.first == keyPiece; });

            if(branch == current_node->children.end())
            {
                current_node->children.emplace_back(keyPiece);

                std::sort(current_node->children.begin(),
                          current_node->children.end(),
                          _compare);

                auto next_node = std::find_if(current_node->children.begin(), current_node->children.end(), 
                                       [&](const node_type& child) { return child.first == keyPiece; });

                current_node = std::addressof(*next_node);
            }
            else
                current_node = std::addressof(*branch);
        }

        bool emplaced = false;
        if(!current_node->second.has_value())
        {
            current_node->second.emplace(std::move(value));
            emplaced = true;
            ++_size;

            if(_compare(*_rightmost, *current_node))
                _rightmost = current_node;
        }

        return std::make_pair(iterator(current_node),emplaced);
    }

    // TODO: return iterator
    const node_type* find(const key_type& key) const
    {
        const node_type* current_node = &_root;
        for(size_t i = 1; i <= key.size(); ++i)
        {
            key_type keyPiece = key.substr(0,i);

            auto branch = std::find_if(current_node->children.begin(), current_node->children.end(), 
                                       [&](const node_type& child) { return child.first == keyPiece; });

            if(branch == current_node->children.end())
                return nullptr;
            else
                current_node = std::addressof(*branch);
        }
        return current_node;
    }

    size_t count(const key_type& key) const
    {
        return (find(key) == nullptr) ? 0 : 1;
    }

private:

    size_t _size;    

    node_type  _root;
    node_type* _rightmost; // TODO: copy ctorban a régire fog mutatni :S

    node_compare _compare;
};

#endif /* STUPID_TRIE__H */