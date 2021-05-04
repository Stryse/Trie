#ifndef GENERIC_TRIE__H
#define GENERIC_TRIE__H

#include <utility>
#include <optional>
#include <vector>
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <stack>

template<typename _Key_Piece,
         typename _Tp,
         typename _Concat,
         template <typename> class _Compare = std::less,
         template <typename,
                   typename,
                   typename> class _Key     = std::basic_string,
         template <typename> class _Traits  = std::char_traits,
         template <typename> class _Alloc   = std::allocator>

class trie
{

protected:
    class  trie_node;
    struct node_compare;

public:
    class iterator;
    class const_iterator;

public:
    /********************************* Member types **********************************/
    using key_type    = _Key<_Key_Piece, _Traits<_Key_Piece>, _Alloc<_Key_Piece>>;
    using key_compare = _Compare<_Key_Piece>;
    using key_concat  = _Concat;
    using mapped_type = _Tp;
    using value_type  = std::pair<const key_type, mapped_type&>;
    using node_type   = trie_node;
    /*********************************************************************************/

protected:
    /******************************** Member classes ********************************/
    struct trie_node
    {
        using key_piece_t  = _Key_Piece;

        key_piece_t key_piece;
        std::reference_wrapper<const key_compare> compare; // Needed for defining operator ==
                                                           // because we use std::find for trie_node-s

        std::optional<mapped_type> value;
        
        trie_node* parent;
        std::vector<trie_node> children;

    /*********************************** Constructors ****************************************************/

        explicit trie_node(const key_compare& key_compare, 
                           node_type* parent = nullptr) 
            : key_piece{}, compare(key_compare), parent(parent)
        {}

        explicit trie_node(const key_piece_t& key_piece, 
                           const key_compare& key_compare, 
                           node_type* parent = nullptr)
            : key_piece(key_piece), compare(key_compare), parent(parent)
        {}

        explicit trie_node(key_piece_t&& key_piece, 
                           const key_compare& key_compare, 
                           node_type* parent = nullptr)
            : key_piece(std::move(key_piece)), compare(key_compare), parent(parent)
        {}

        explicit trie_node(const key_piece_t& key_piece, 
                           const key_compare& key_compare, 
                           const mapped_type& value, 
                           node_type* parent = nullptr)

            : key_piece(key_piece), compare(key_compare), value(value), parent(parent)
        {}

        explicit trie_node(key_piece_t&& key_piece, 
                           const key_compare& key_compare,
                           mapped_type&& value, 
                           node_type* parent = nullptr)

            : key_piece(std::move(key_piece)), compare(key_compare), 
              value(std::move(value)), parent(parent)
        {}

        virtual ~trie_node() = default;

        trie_node(const trie_node& other)
            : key_piece(other.key_piece), compare(other.compare),
              value(other.value), parent(other.parent) ,children(other.children)
        {
            // Revalidate parent pointers since copy invalidated it
            for(auto& child : children)
                child.parent = this;
        }

        trie_node(trie_node&& other) noexcept
            : key_piece(std::move(other.key_piece)), compare(std::move(other.compare)),
              value(std::move(other.value)), parent(std::move(other.parent)), 
              children(std::move(other.children))
        {
            // Revalidate parent pointers since move invalidated it
            for(auto& child : children)
                child.parent = this;
        }

        /************************************ Assignment ****************************************/
        trie_node& operator=(const trie_node& other)
        {
            this->key_piece    = other.key_piece;
            this->compare      = other.compare;
            this->value        = other.value;
            this->children     = other.children;
            this->parent       = other.parent;

            // Revalidate parent pointers since copy invalidated it
            for(auto& child : children)
                child.parent = this;

            return *this;
        }

        trie_node& operator=(trie_node&& other) noexcept
        {
            this->key_piece    = std::move(other.key_piece);
            this->compare      = std::move(other.compare);
            this->value        = std::move(other.value);
            this->children     = std::move(other.children);
            this->parent       = std::move(other.parent);

            // Revalidate parent pointers since move invalidated it
            for(auto& child : children)
                child.parent = this;

            return *this;
        }

        /****************************************** Functionality *********************************/
        bool operator==(const node_type& other) const
        {
            return !compare(key_piece,other.key_piece) && !compare(other.key_piece,key_piece);
        }

        const node_type* next_node() const
        {
            const node_type* current_node = this;

            // True -> We can't go deeper the tree
            if(current_node->children.empty() && current_node->parent != nullptr)
            {
                auto next_sibling = ++std::find(current_node->parent->children.begin(), 
                                                current_node->parent->children.end(), 
                                                *current_node);

                // Going up while we are the last child
                while(next_sibling == current_node->parent->children.end())
                {
                    current_node = current_node->parent;

                    // There's nowhere to move if node has no parent nor sibling
                    if(current_node->parent == nullptr)
                        return nullptr;

                    next_sibling = ++std::find(current_node->parent->children.begin(), 
                                               current_node->parent->children.end(), 
                                               *current_node);
                }

                current_node = std::addressof(*next_sibling);
            }
            // There is no next node if root (node with no parent) has no children
            else if(current_node->children.empty() && current_node->parent == nullptr)
                return nullptr;
            // If node has child we select that branch
            else
                current_node = &this->children.front();

            // Expanding first child until we have one with value
            while(!current_node->value.has_value())
                current_node = &current_node->children.front();

            return current_node;
        }

        node_type* next_node()
        {
            return const_cast<node_type*>(static_cast<const trie_node*>(this)->next_node());
        }

        key_type trace_key(const key_concat& concat) const
        {
            const trie_node* current_node = this;
            std::stack<key_piece_t> reversed_key;

            while(current_node->parent != nullptr)
            {
                reversed_key.emplace(current_node->key_piece);
                current_node = current_node->parent;
            }

            key_type key;
            while(!reversed_key.empty())
            {
                concat(key,reversed_key.top());
                reversed_key.pop();
            }
            return key;
        }
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
            return compare(lhs.key_piece,rhs.key_piece);
        }

        const key_compare& compare;
    };    

public:
    /***************************************** Iterator *******************************************/
    class iterator
    {
        friend class const_iterator;

    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = trie::value_type;
        using pointer           = std::unique_ptr<value_type>;
        
        explicit iterator(node_type* ptr, const key_concat& concat) 
            :  _pointed_node(ptr), _concat(concat) {}


        iterator(const iterator&)            = default;
        iterator(iterator&&) noexcept        = default;
        virtual ~iterator()                  = default;

        iterator& operator=(const iterator&)     = default;
        iterator& operator=(iterator&&) noexcept = default;

        value_type operator* () const 
        { 
            return value_type(_pointed_node->trace_key(_concat), _pointed_node->value.value()); 
        }

        pointer operator->() const 
        { 
            return std::make_unique<value_type>(_pointed_node->trace_key(_concat),_pointed_node->value.value()); 
        }
        
        iterator& operator++()
        {
            _pointed_node = _pointed_node->next_node();
            return *this;
        }

        iterator operator++(int)
        {
            iterator no_op = *this;
            ++(*this);
            return no_op;
        }

        friend bool operator==(const iterator& lhs, const iterator& rhs) { return lhs._pointed_node == rhs._pointed_node; }
        friend bool operator!=(const iterator& lhs, const iterator& rhs) { return !(lhs == rhs); }

    private:
        node_type* _pointed_node;
        const key_concat& _concat;
    };

    class const_iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using value_type        = trie::value_type;
        using pointer           = std::unique_ptr<value_type>;

        explicit const_iterator(const node_type* ptr, const key_concat& concat) 
            : _pointed_node(ptr), _concat(concat) {}

        // We are allowing implicit conversion from iterator -> const interator
        const_iterator(const iterator& it)
            : _pointed_node(it._pointed_node), 
              _concat(it._concat) {}


        virtual ~const_iterator()                        = default;
        const_iterator(const const_iterator&)            = default;
        const_iterator(const_iterator&&) noexcept        = default;

        const_iterator& operator=(const const_iterator&)     = default;
        const_iterator& operator=(const_iterator&&) noexcept = default;

        const value_type operator* () const 
        { 
            return value_type(_pointed_node->trace_key(_concat), _pointed_node->value.value()); 
        }

        const pointer operator->() const 
        { 
            return std::make_unique<value_type>(_pointed_node->trace_key(_concat),_pointed_node->value.value()); 
        }
        
        iterator& operator++()
        {
            _pointed_node = _pointed_node->next_node();
            return *this;
        }

        iterator operator++(int)
        {
            iterator no_op = *this;
            ++(*this);
            return no_op;
        }

        friend bool operator==(const const_iterator& lhs, const const_iterator& rhs) { return lhs._pointed_node == rhs._pointed_node; }
        friend bool operator!=(const const_iterator& lhs, const const_iterator& rhs) { return !(lhs == rhs); }

    private:
        const node_type* _pointed_node;
        const key_concat& _concat;
    };

    iterator begin() noexcept
    {
        node_type* current_node = &_root;

        while(!current_node->children.empty())
            current_node = &current_node->children.front();

        return (current_node->value.has_value()) ? iterator(current_node, _key_concat) : end();
    }

    const_iterator begin()  const noexcept { return cbegin(); }

    iterator       end()          noexcept { return iterator(nullptr, _key_concat); }
    const_iterator end()    const noexcept { return cend();            }

    const_iterator cbegin() const noexcept { return const_iterator(begin());   }
    const_iterator cend()   const noexcept { return const_iterator(nullptr, _key_concat);   }

private:

    const node_type* find_node(const key_type& key) const
    {
        const node_type* current_node = &_root;
        for(const auto& key_piece : key)
        {
            auto branch = std::find_if(current_node->children.begin(), current_node->children.end(), 
                                       [&](const node_type& child) { return child.key_piece == key_piece; });

            if(branch == current_node->children.end())
                return nullptr;
            else
                current_node = std::addressof(*branch);
        }
        return current_node;
    }

    node_type* find_node(const key_type& key)
    {
        return const_cast<node_type*>(static_cast<const trie*>(this)->find_node(key));
    }

public:
    /********************************* Constructors **********************************/
    explicit trie(const key_concat&  concat,
                  const key_compare& compare = key_compare{})

        : _size{0}, _key_concat{concat}, _key_compare{compare},
          _node_compare{compare}, _root{_key_compare}
    {}

    trie(const trie&)     = default;
    trie(trie&&) noexcept = default;
    virtual ~trie()       = default;

    /****************************** Assignment operators *****************************/

    trie& operator=(const trie& other) = default;
    trie& operator=(trie&&) noexcept   = default;

    /*********************************************************************************/

    bool   empty() const noexcept { return _size == 0;  }
    size_t size()  const noexcept { return _size;       } 

    size_t count(const key_type& key) const
    {
        return (find(key) == cend()) ? 0 : 1;
    }

    /***************************************
     * Invalidates all previous iterators !!
    ****************************************/
    template<typename Key, typename Value>
    std::pair<iterator,bool> emplace(Key&& key, Value&& value)
    {
        node_type* current_node = &_root;

        for(auto& key_piece : key_type(std::forward<Key>(key)))
        {
            // Lookup branch
            auto branch = std::find_if(current_node->children.begin(), current_node->children.end(), 
                                       [&](const node_type& child) { return !_key_compare(child.key_piece,key_piece) &&
                                                                            !_key_compare(key_piece,child.key_piece);});

            // We need new branch if it doesn't exist
            if(branch == current_node->children.end())
            {
                current_node->children.emplace_back(key_piece, _key_compare ,current_node);

                // Sort children to regain search tree invariant
                std::sort(current_node->children.begin(),
                          current_node->children.end(),
                          _node_compare);

                // Locate next node since sorting might moved it
                auto next_node = std::find_if(current_node->children.begin(), current_node->children.end(), 
                                       [&](const node_type& child) { return !_key_compare(child.key_piece,key_piece) &&
                                                                            !_key_compare(key_piece,child.key_piece);});

                current_node = std::addressof(*next_node);
            }
            else
                current_node = std::addressof(*branch);
        }

        bool emplaced = false;
        if(!current_node->value.has_value())
        {
            current_node->value.emplace(std::forward<Value>(value));
            emplaced = true;
            ++_size;
        }

        return std::make_pair(iterator(current_node, _key_concat),emplaced);
    }

    iterator find(const key_type& key)
    {
        node_type* target = find_node(key);
        return (target != nullptr && target->value.has_value()) ? iterator(target, _key_concat) : end();
    }

    const_iterator find(const key_type& key) const
    {
        const node_type* target = find_node(key);
        return (target != nullptr && target->value.has_value()) ? const_iterator(target, _key_concat) : cend();
    }

    mapped_type& at(const key_type& key)
    {
        node_type* target = find_node(key);
        
        if(target != nullptr && target->value.has_value())
            return target->value.value();
        else
            throw std::out_of_range("trie::at() was invoked with key that is not stored.");
    }

    const mapped_type& at(const key_type& key) const
    {
        const node_type* target = find_node(key);
        
        if(target != nullptr && target->value.has_value())
            return target->value.value();
        else
            throw std::out_of_range("trie::at() was invoked with key that is not stored.");
    }

    std::optional<std::reference_wrapper<mapped_type>> operator[](const key_type& key)
    {
        node_type* target = find_node(key);

        return (target != nullptr && target->value.has_value())
                    ? std::optional(std::ref(target->value.value())) : std::nullopt;
    }

    const std::optional<std::reference_wrapper<const mapped_type>> operator[](const key_type& key) const
    {
        const node_type* target = find_node(key);

        return (target != nullptr && target->value.has_value())
                    ? std::optional(std::cref(target->value.value())) : std::nullopt;
    }

private:

    size_t _size;    
    key_concat   _key_concat;
    key_compare  _key_compare;
    node_compare _node_compare;
    node_type    _root;
};

#endif /* GENERIC_TRIE__H */