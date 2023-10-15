#pragma once

#include <iterator>

template <typename T, typename Alloc = std::allocator<T>>
class List : private Alloc {
  private:
    struct BaseNode {
        BaseNode* next{nullptr};
        BaseNode* prev{nullptr};
        BaseNode(BaseNode* next, BaseNode* prev)
            : next(next), prev(prev) {}
        BaseNode() = default;
    };
    struct Node : BaseNode {
        T value;
        Node() = default;
        Node(const T& value)
            : value(value) {}
    };
    using AllocTraits = std::allocator_traits<Alloc>;
    using NodeAllocator = typename AllocTraits::template rebind_alloc<Node>;

    BaseNode head_;
    size_t size_{0};
    NodeAllocator node_allocator_{get_allocator()};
    using NodeAllocTraits = std::allocator_traits<NodeAllocator>;

  public:
    template <bool IsConst>
    struct common_iterator;
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    template <bool IsConst>
    struct common_iterator {
        using value_type = std::conditional_t<IsConst, const T, T>;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using iterator_category = std::bidirectional_iterator_tag;

        common_iterator() = delete;

        common_iterator(const BaseNode* node)
            : node_(const_cast<BaseNode*>(node)) {}  // NOLINT

        reference operator*() const {
            return static_cast<Node*>(node_)->value;
        }

        pointer operator->() const {
            return &**this;
        }

        common_iterator operator++() {
            node_ = node_->next;
            return *this;
        }

        common_iterator operator--() {
            node_ = node_->prev;
            return *this;
        }

        common_iterator operator++(int) {
            common_iterator copy = *this;
            ++(*this);
            return copy;
        }

        common_iterator operator--(int) {
            common_iterator copy = *this;
            --(*this);
            return copy;
        }

        bool operator==(const common_iterator& other) const {
            return node_ == other.node_;
        }

        operator const_iterator() const {
            return {node_};
        }

      private:
        friend List;
        BaseNode* node_;
    };

    const_iterator cbegin() const {
        return const_iterator{head_.next};
    }

    iterator begin() {
        return iterator{head_.next};
    }

    const_iterator begin() const {
        return cbegin();
    }

    const_reverse_iterator crbegin() const {
        return const_reverse_iterator(end());
    }

    reverse_iterator rbegin() {
        return reverse_iterator(end());
    }

    const_reverse_iterator rbegin() const {
        return crbegin();
    }

    const_iterator cend() const {
        return const_iterator{&head_};
    }

    iterator end() {
        return iterator{&head_};
    }

    const_iterator end() const {
        return cend();
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator{begin()};
    }

    reverse_iterator rend() {
        return reverse_iterator(begin());
    }

    const_reverse_iterator rend() const {
        return crend();
    }

    Alloc get_allocator() const {
        return static_cast<Alloc>(*this);
    }

    List() {
        initialize_head();
    }

    List(const Alloc& alloc)
        : Alloc(alloc) {
        initialize_head();
    }

    List(size_t count, const Alloc& alloc)
        : List(alloc) {
        size_t i;
        try {
            for (i = 0; i < count; ++i) {
                emplace(end());
            }
        } catch (...) {
            destroy(i);
            throw;
        }
    }

    List(size_t count, const T& obj, const Alloc& alloc)
        : List(alloc) {
        size_t i;
        try {
            for (i = 0; i < count; ++i) {
                push_back(obj);
            }
        } catch (...) {
            destroy(i);
            throw;
        }
    }

    List(const List& other)
        : List(AllocTraits::select_on_container_copy_construction(
              static_cast<const Alloc&>(other))) {
        size_t i = 0;
        try {
            for (const auto& elem : other) {
                push_back(elem);
                ++i;
            }
        } catch (...) {
            destroy(i);
            throw;
        }
    }

    List(size_t count)
        : List(count, get_allocator()) {}

    List(size_t count, const T& obj)
        : List(count, obj, get_allocator()) {}

    ~List() {
        destroy(size_);
    }

    List& operator=(const List& other) {
        List another(AllocTraits::propagate_on_container_copy_assignment::value
                         ? other.get_allocator()
                         : get_allocator());
        for (const auto& i : other) {
            another.push_back(i);
        }
        swap(another);
        return *this;
    }

    size_t size() const {
        return size_;
    }

    void push_back(const T& obj) {
        insert(end(), obj);
    }

    void push_front(const T& obj) {
        insert(begin(), obj);
    }

    void pop_back() {
        erase(--end());
    }

    void pop_front() {
        erase(begin());
    }

    void insert(const_iterator it, const T& obj) {
        emplace(it, obj);
    }

    void erase(const_iterator it) {
        it.node_->prev->next = it.node_->next;
        it.node_->next->prev = it.node_->prev;
        NodeAllocTraits::destroy(node_allocator_, static_cast<Node*>(it.node_));
        NodeAllocTraits::deallocate(node_allocator_,
                                    static_cast<Node*>(it.node_), 1);
        --size_;
    }

  private:
    void initialize_head() {
        head_ = {&head_, &head_};
    }

    void swap(List<T, Alloc>& other) {
        std::swap(static_cast<Alloc&>(*this), static_cast<Alloc&>(other));
        std::swap(size_, other.size_);
        std::swap(node_allocator_, other.node_allocator_);
        std::swap(head_, other.head_);
        std::swap(head_.next->prev, other.head_.next->prev);
        std::swap(head_.prev->next, other.head_.prev->next);
    }

    template <typename... Args>
    void emplace(const_iterator it, Args&&... args) {
        Node* node = NodeAllocTraits::allocate(node_allocator_, 1);
        try {
            NodeAllocTraits::construct(node_allocator_, node, std::forward<Args>(args)...);
        } catch (...) {
            NodeAllocTraits::deallocate(node_allocator_, node, 1);
            throw;
        }
        BaseNode* prev = it.node_->prev;
        BaseNode* next = it.node_;
        next->prev = node;
        node->next = next;
        prev->next = node;
        node->prev = prev;
        ++size_;
    }

    void destroy(size_t d) {
        for (size_t j = 0; j < d; ++j) {
            pop_back();
        }
    }
};
