#ifndef STUPID_TRIE__H
#define STUPID_TRIE__H

#include <string>
#include <utility>
#include <optional>
#include <vector>
#include <algorithm>
#include <memory>
#include <stdexcept>

template<typename _Tp, 
         typename _Compare = std::less<std::string>>

class stupid_trie
{

protected:
    class trie_node;

public:
    class iterator;
    class const_iterator;

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
        explicit node_compare(const key_compare& compare)
            : compare(compare){}

        bool operator()(const node_type& lhs, const node_type& rhs) const
        {
            return compare(lhs.first,rhs.first);
        }

        key_compare compare;
    };

public:
    /***************************************** Iterator *******************************************/
    class iterator
    {
        friend class const_iterator;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::pair<const key_type, const std::reference_wrapper<mapped_type>>;
        
        explicit iterator(node_type* ptr) : _pointed_node(ptr) {}
        ~iterator()               = default;
        iterator(const iterator&) = default;
        iterator(iterator&&)      = default;

        iterator& operator=(const iterator&) = default;
        iterator& operator=(iterator&&)      = default;

        value_type operator* () const 
        { 
            return value_type(_pointed_node->first, _pointed_node->second.value()); 
        }

        std::unique_ptr<value_type> operator->() const 
        { 
            return std::make_unique<value_type>(_pointed_node->first,_pointed_node->second.value()); 
        }
        
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

    class const_iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = std::pair<const key_type, const std::reference_wrapper<const mapped_type>>;

        explicit const_iterator(const node_type* ptr) : _pointed_node(ptr) {}
        const_iterator(const iterator& it)            : _pointed_node(it._pointed_node) {}

        ~const_iterator()                     = default;
        const_iterator(const const_iterator&) = default;
        const_iterator(const_iterator&&)      = default;

        const_iterator& operator=(const const_iterator&) = default;
        const_iterator& operator=(const_iterator&&)      = default;

        value_type operator* () const 
        { 
            return value_type(_pointed_node->first, _pointed_node->second.value()); 
        }

        std::unique_ptr<value_type> operator->() const 
        { 
            return std::make_unique<value_type>(_pointed_node->first,_pointed_node->second.value()); 
        }
        
        //iterator& operator++()
        //{

        //}

        //iterator operator++(int)
        //{

        //}

        friend bool operator==(const const_iterator& lhs, const const_iterator& rhs) { return lhs._pointed_node == rhs._pointed_node; }
        friend bool operator!=(const const_iterator& lhs, const const_iterator& rhs) { return !(lhs == rhs); }

    private:
        const node_type* _pointed_node;
    };

    iterator begin() const noexcept { return iterator(&_root);  }
    iterator end()   const noexcept { return iterator(nullptr); }

    const_iterator cbegin() const noexcept { return const_iterator(&_root);  }
    const_iterator cend()   const noexcept { return const_iterator(nullptr); }

private:

    const node_type* find_node(const key_type& key) const
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

public:
    /********************************* Constructors **********************************/
    explicit stupid_trie(const _Compare& compare = _Compare{}) 
        : _size{}, _rightmost{&_root}, _compare{compare}
    {}

    stupid_trie(const stupid_trie&) = default;
    stupid_trie(stupid_trie&&)      = default;
    virtual ~stupid_trie()          = default;
    /*********************************************************************************/

    bool   empty() const noexcept { return _size == 0;  }
    size_t size()  const noexcept { return _size;       } 

    size_t count(const key_type& key) const
    {
        return (find(key) == cend()) ? 0 : 1;
    }

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

    iterator find(const key_type& key)
    {
        return iterator(const_cast<node_type*>(find_node(key)));
    }

    const_iterator find(const key_type& key) const
    {
        return const_iterator(find_node(key));
    }

    const mapped_type& at(const key_type& key) const
    {
        const node_type* target = find_node(key);
        
        if(target != nullptr && target->second.has_value())
            return target->second.value();
        else
            throw std::out_of_range("stupid_trie::at() was invoked with key that is not stored.");
    }

    mapped_type& at(const key_type& key)
    {
        node_type* target = const_cast<node_type*>(find_node(key));
        
        if(target != nullptr && target->second.has_value())
            return target->second.value();
        else
            throw std::out_of_range("stupid_trie::at() was invoked with key that is not stored.");
    }

    const std::optional<mapped_type> operator[](const key_type& key) const
    {
        const node_type* target = find_node(key);

        if(target == nullptr)
            return std::nullopt;
        else
            return target->second;
    }

    std::optional<mapped_type> operator[](const key_type& key)
    {
        node_type* target = const_cast<node_type*>(find_node(key));

        if(target == nullptr)
            return std::nullopt;
        else
            return target->second;
    }

private:

    size_t _size;    

    node_type  _root;
    node_type* _rightmost; // TODO: copy ctorban a régire fog mutatni :S

    node_compare _compare;
};

#endif /* STUPID_TRIE__H */