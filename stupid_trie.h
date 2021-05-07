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
    struct node_compare;

public:
    class iterator;
    class const_iterator;

public:
    /********************************* Member types *********************************/
    using key_type        = std::string;
    using mapped_type     = _Tp;
    using value_type      = std::pair<const std::string, mapped_type&>;
    using size_type       = size_t;
    using key_compare     = _Compare;
    using node_type       = trie_node;
    /*********************************************************************************/

protected:
    /******************************** Member classes ********************************/
    struct trie_node
    {
        key_type first;
        std::reference_wrapper<const key_compare> compare;
        std::optional<mapped_type> second;

        trie_node* parent;
        std::vector<trie_node> children;

        /*********************************** Constructors ****************************************************/
        
        explicit trie_node(const key_compare& key_compare, node_type* parent = nullptr) 
            : first{}, compare(key_compare), parent(parent)
        {}

        explicit trie_node(const key_type& key, const key_compare& key_compare, node_type* parent = nullptr)
            : first(key), compare(key_compare), parent(parent)
        {}

        explicit trie_node(key_type&& key, const key_compare& key_compare, node_type* parent = nullptr)
            : first(std::move(key)), compare(key_compare), parent(parent)
        {}

        explicit trie_node(const key_type& key, const key_compare& key_compare, 
                           const mapped_type& value, node_type* parent = nullptr)

            : first(key), compare(key_compare), second(value), parent(parent)
        {}

        explicit trie_node(key_type&& key, const key_compare& key_compare,
                           mapped_type&& value, node_type* parent = nullptr)

            : first(std::move(key)), compare(key_compare), second(std::move(value)), parent(parent)
        {}

        virtual ~trie_node() = default;

        trie_node(const trie_node& other)
            : first(other.first), compare(other.compare),
              second(other.second), parent(other.parent) ,children(other.children)
        {
            // Revalidate parent pointers since copy invalidated it
            for(auto& child : children)
                child.parent = this;
        }

        trie_node(trie_node&& other) noexcept
            : first(std::move(other.first)), compare(std::move(other.compare)),
              second(std::move(other.second)), parent(std::move(other.parent)), 
              children(std::move(other.children))
        {
            // Revalidate parent pointers since move invalidated it
            for(auto& child : children)
                child.parent = this;
        }

        /************************************ Assignment ****************************************/
        trie_node& operator=(const trie_node& other)
        {
            this->first    = other.first;
            this->compare  = other.compare;
            this->second   = other.second;
            this->children = other.children;
            this->parent   = other.parent;

            // Revalidate parent pointers since copy invalidated it
            for(auto& child : children)
                child.parent = this;

            return *this;
        }

        trie_node& operator=(trie_node&& other) noexcept
        {
            this->first    = std::move(other.first);
            this->compare  = std::move(other.compare);
            this->second   = std::move(other.second);
            this->children = std::move(other.children);
            this->parent   = std::move(other.parent);

            // Revalidate parent pointers since move invalidated it
            for(auto& child : children)
                child.parent = this;

            return *this;
        }

        /****************************************** Functionality *********************************/
        bool operator==(const node_type& other) const
        {
            return !compare(first,other.first) && !compare(other.first,first);
        }

        const node_type* next_node() const
        {
            const node_type* current_node = this;

            // True -> We can't go deeper the tree
            if(current_node->children.empty() && current_node->parent != nullptr)
            {
                // Going up while we are the last child
                while(current_node == &current_node->parent->children.back())
                {
                    current_node = current_node->parent;

                    // There's nowhere to move if node has no parent nor sibling
                    if(current_node->parent == nullptr)
                        return nullptr;
                }

                auto next_sibling = ++std::find(current_node->parent->children.begin(), 
                                                current_node->parent->children.end(), 
                                                *current_node);

                current_node = std::addressof(*next_sibling);
            }
            // There is no next node if root (node with no parent) has no children
            else if(current_node->children.empty() && current_node->parent == nullptr)
                return nullptr;
            // If node has child we select that branch
            else
                current_node = &this->children.front();

            // Expanding first child until we have one with value
            while(!current_node->second.has_value())
                current_node = &current_node->children.front();

            return current_node;
        }

        node_type* next_node()
        {
            return const_cast<node_type*>(static_cast<const trie_node*>(this)->next_node());
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
            return compare(lhs.first,rhs.first);
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
        using value_type        = stupid_trie::value_type;
        using pointer           = std::unique_ptr<value_type>;
        
        explicit iterator(node_type* ptr) :  _pointed_node(ptr) {}
        iterator(const iterator&)            = default;
        iterator(iterator&&) noexcept        = default;
        virtual ~iterator()                  = default;

        iterator& operator=(const iterator&)     = default;
        iterator& operator=(iterator&&) noexcept = default;

        value_type operator* () const 
        { 
            return value_type(_pointed_node->first, _pointed_node->second.value()); 
        }

        pointer operator->() const 
        { 
            return std::make_unique<value_type>(_pointed_node->first,_pointed_node->second.value()); 
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
    };

    class const_iterator
    {
    public:
        using iterator_category = std::forward_iterator_tag;
        using difference_type   = std::ptrdiff_t;
        using const_value_type  = std::pair<const key_type, const mapped_type&>;
        using pointer           = std::unique_ptr<const_value_type>;

        explicit const_iterator(const node_type* ptr) : _pointed_node(ptr) {}

        // We are allowing implicit conversion from iterator -> const interator
        const_iterator(const iterator& it)            : _pointed_node(it._pointed_node) {}

        virtual ~const_iterator()                        = default;
        const_iterator(const const_iterator&)            = default;
        const_iterator(const_iterator&&) noexcept        = default;

        const_iterator& operator=(const const_iterator&)     = default;
        const_iterator& operator=(const_iterator&&) noexcept = default;

        const_value_type operator* () const 
        { 
            return value_type(_pointed_node->first, _pointed_node->second.value()); 
        }

        const pointer operator->() const 
        { 
            return std::make_unique<const_value_type>(_pointed_node->first,_pointed_node->second.value()); 
        }
        
        const_iterator& operator++()
        {
            _pointed_node = _pointed_node->next_node();
            return *this;
        }

        const_iterator operator++(int)
        {
            const_iterator no_op = *this;
            ++(*this);
            return no_op;
        }

        friend bool operator==(const const_iterator& lhs, const const_iterator& rhs) { return lhs._pointed_node == rhs._pointed_node; }
        friend bool operator!=(const const_iterator& lhs, const const_iterator& rhs) { return !(lhs == rhs); }

    private:
        const node_type* _pointed_node;
    };

    iterator begin() noexcept
    {
        node_type* current_node = &_root;

        while(!current_node->second.has_value())
            current_node = &current_node->children.front();

        return (current_node->second.has_value()) ? iterator(current_node) : end();
    }

    const_iterator begin()  const noexcept 
    { 
        const node_type* current_node = &_root;

        while(!current_node->second.has_value())
            current_node = &current_node->children.front();

        return (current_node->second.has_value()) ? iterator(current_node) : end();
    }

    iterator       end()          noexcept { return iterator(nullptr);         }
    const_iterator end()    const noexcept { return const_iterator(nullptr);   }

    const_iterator cbegin() const noexcept { return begin();   }
    const_iterator cend()   const noexcept { return end();     }

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

    node_type* find_node(const key_type& key)
    {
        return const_cast<node_type*>(static_cast<const stupid_trie*>(this)->find_node(key));
    }

public:
    /********************************* Constructors **********************************/
    explicit stupid_trie(const key_compare& compare = key_compare{}) 
        : _size{0}, _key_compare{compare}, _node_compare{compare}, _root{_key_compare}
    {}

    stupid_trie(const stupid_trie&)     = default;
    stupid_trie(stupid_trie&&) noexcept = default;
    virtual ~stupid_trie()              = default;

    /****************************** Assignment operators *****************************/

    stupid_trie& operator=(const stupid_trie& other) = default;
    stupid_trie& operator=(stupid_trie&&) noexcept   = default;

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
        key_type local_key(std::forward<Key>(key));
        node_type* current_node = &_root;

        for(size_t i = 1; i <= local_key.size(); ++i)
        {
            key_type keyPiece = local_key.substr(0,i);

            // Lookup branch
            auto branch = std::find_if(current_node->children.begin(), current_node->children.end(), 
                                       [&](const node_type& child) { return !_key_compare(child.first,keyPiece) &&
                                                                            !_key_compare(keyPiece,child.first); });

            // We need new branch if it doesn't exist
            if(branch == current_node->children.end())
            {
                current_node->children.emplace_back(keyPiece, _key_compare ,current_node);

                // Sort children to regain search tree invariant
                std::sort(current_node->children.begin(),
                          current_node->children.end(),
                          _node_compare);

                // Locate next node since sorting might moved it
                auto next_node = std::find_if(current_node->children.begin(), current_node->children.end(), 
                                       [&](const node_type& child) { return !_key_compare(child.first,keyPiece) &&
                                                                            !_key_compare(keyPiece,child.first); });

                current_node = std::addressof(*next_node);
            }
            else
                current_node = std::addressof(*branch);
        }

        bool emplaced = false;
        if(!current_node->second.has_value())
        {
            current_node->second.emplace(std::forward<Value>(value));
            emplaced = true;
            ++_size;
        }

        return std::make_pair(iterator(current_node),emplaced);
    }

    iterator find(const key_type& key)
    {
        node_type* target = find_node(key);
        return (target != nullptr && target->second.has_value()) ? iterator(target) : end();
    }

    const_iterator find(const key_type& key) const
    {
        const node_type* target = find_node(key);
        return (target != nullptr && target->second.has_value()) ? const_iterator(target) : cend();
    }

    mapped_type& at(const key_type& key)
    {
        node_type* target = find_node(key);
        
        if(target != nullptr && target->second.has_value())
            return target->second.value();
        else
            throw std::out_of_range("stupid_trie::at() was invoked with key that is not stored.");
    }

    const mapped_type& at(const key_type& key) const
    {
        const node_type* target = find_node(key);
        
        if(target != nullptr && target->second.has_value())
            return target->second.value();
        else
            throw std::out_of_range("stupid_trie::at() was invoked with key that is not stored.");
    }

    std::optional<std::reference_wrapper<mapped_type>> operator[](const key_type& key)
    {
        node_type* target = find_node(key);

        return (target != nullptr && target->second.has_value())
                    ? std::optional(std::ref(target->second.value())) : std::nullopt;
    }

    const std::optional<std::reference_wrapper<const mapped_type>> operator[](const key_type& key) const
    {
        const node_type* target = find_node(key);

        return (target != nullptr && target->second.has_value())
                    ? std::optional(std::cref(target->second.value())) : std::nullopt;
    }

private:

    size_t _size;    
    key_compare  _key_compare;
    node_compare _node_compare;
    node_type  _root;
};

#endif /* STUPID_TRIE__H */