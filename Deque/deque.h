#pragma once

#include <iterator>

template <typename T>
class Deque {
  public:
    template <bool IsConst>
    struct common_iterator;
    using iterator = common_iterator<false>;
    using const_iterator = common_iterator<true>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    template <bool IsConst>
    struct common_iterator {
      public:
        using value_type = std::conditional_t<IsConst, const T, T>;
        using difference_type = std::ptrdiff_t;
        using reference = std::conditional_t<IsConst, const T&, T&>;
        using pointer = std::conditional_t<IsConst, const T*, T*>;
        using iterator_category = std::random_access_iterator_tag;
        common_iterator() = delete;
        common_iterator(T** ptr, size_t index)
            : ptr_(ptr), array_(ptr_[index / BLOCK_SIZE_]), index_(index) {}

        reference operator*() const {
            return array_[index_ % BLOCK_SIZE_];
        }

        pointer operator->() const {
            return &**this;
        }

        difference_type operator-(const common_iterator& it) const {
            return index_ - it.index_ + (ptr_ - it.ptr_) * BLOCK_SIZE_;
        }

        common_iterator& operator++() {
            return *this += 1;
        }

        common_iterator& operator--() {
            return *this -= 1;
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

        common_iterator& operator+=(const difference_type& diff) {
            if (diff < 0) {
                return *this -= (-diff);
            }
            index_ += diff;
            array_ = ptr_[index_ / BLOCK_SIZE_];
            return *this;
        }

        common_iterator& operator-=(const difference_type& diff) {
            if (diff < 0) {
                return *this += (-diff);
            }
            index_ -= diff;
            array_ = ptr_[index_ / BLOCK_SIZE_];
            return *this;
        }

        common_iterator operator+(const difference_type& diff) const {
            common_iterator copy(*this);
            copy += diff;
            return copy;
        }

        common_iterator operator-(const difference_type& diff) const {
            common_iterator copy(*this);
            copy -= diff;
            return copy;
        }

        bool operator<(const common_iterator& other) const {
            return index_ < other.index_;
        }

        bool operator>(const common_iterator& other) const {
            return other < *this;
        }

        bool operator<=(const common_iterator& other) const {
            return !((*this) > other);
        }

        bool operator>=(const common_iterator& other) const {
            return other <= *this;
        }

        bool operator==(const common_iterator& other) const {
            return index_ == other.index_;
        }

        operator const_iterator() {
            return {ptr_, array_, index_};
        }

      private:
        T** ptr_;
        T* array_;
        size_t index_;
    };

    const_iterator cbegin() const {
        return const_iterator{
            external_array_, first_block_ * BLOCK_SIZE_ + first_element_index_};
    }

    iterator begin() {
        return iterator{external_array_,
                        first_block_ * BLOCK_SIZE_ + first_element_index_};
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
        return const_iterator{external_array_, last_block_ * BLOCK_SIZE_ +
                                                   after_last_element_index_};
    }

    iterator end() {
        return iterator{external_array_,
                        last_block_ * BLOCK_SIZE_ + after_last_element_index_};
    }

    const_iterator end() const {
        return cend();
    }

    const_reverse_iterator crend() const {
        return const_reverse_iterator{begin()};
    }

    reverse_iterator rend() {
        return reverse_iterator{begin()};
    }

    const_reverse_iterator rend() const {
        return crend();
    }

    size_t size() const {
        return (last_block_ - first_block_) * BLOCK_SIZE_ -
               first_element_index_ + after_last_element_index_;
    }

    Deque()
        : amount_of_blocks_(DEFAULT_CAPACITY_),
          external_array_(new T*[amount_of_blocks_]),
          first_block_(amount_of_blocks_ / 2),
          first_element_index_(0),
          last_block_(amount_of_blocks_ / 2),
          after_last_element_index_(0) {
        try_allocate(external_array_, amount_of_blocks_);
    }

    Deque(const Deque<T>& other)
        : amount_of_blocks_(other.amount_of_blocks_),
          external_array_(new T*[amount_of_blocks_]),
          first_block_(other.first_block_),
          first_element_index_(other.first_element_index_),
          last_block_(other.last_block_),
          after_last_element_index_(other.after_last_element_index_) {
        try_allocate(external_array_, amount_of_blocks_);
        size_t i;
        try {
            for (i = 0; i < size(); ++i) {
                new (&(*this)[i]) T(other[i]);
            }
        } catch (...) {
            destroy(i);
            deallocate();
            throw;
        }
    }

    explicit Deque(size_t num)
        : amount_of_blocks_(
              std::max(3 * num / BLOCK_SIZE_ + 1, DEFAULT_CAPACITY_ * 1ul)),
          external_array_(new T*[amount_of_blocks_]),
          first_block_(amount_of_blocks_ / 2 - num / (2 * BLOCK_SIZE_)),
          first_element_index_(0),
          last_block_(first_block_ + num / BLOCK_SIZE_),
          after_last_element_index_(num % BLOCK_SIZE_) {
        try_allocate(external_array_, amount_of_blocks_);
        size_t i;
        try {
            for (i = 0; i < size(); ++i) {
                new (&(*this)[i]) T();
            }
        } catch (...) {
            destroy(i);
            deallocate();
            throw;
        }
    }
    Deque(size_t num, const T& obj)
        : amount_of_blocks_(3 * num / BLOCK_SIZE_ + 1),
          external_array_(new T*[amount_of_blocks_]),
          first_block_(amount_of_blocks_ / 2 - num / (2 * BLOCK_SIZE_)),
          first_element_index_(0),
          last_block_(first_block_ + num / BLOCK_SIZE_),
          after_last_element_index_(num % BLOCK_SIZE_) {
        try_allocate(external_array_, amount_of_blocks_);
        size_t i;
        try {
            for (i = 0; i < size(); ++i) {
                new (&(*this)[i]) T(obj);
            }
        } catch (...) {
            destroy(i);
            deallocate();
            throw;
        }
    }

    Deque& operator=(Deque other) {
        swap(other);
        return *this;
    }

    ~Deque() {
        destroy(size());
        deallocate();
    }

    T& operator[](size_t index) {
        return *(begin() + index);
    }

    const T& operator[](size_t index) const {
        return *(begin() + index);
    }

    T& at(size_t index) {
        if (index >= size()) {
            throw std::out_of_range("You're out of range =(");
        }
        return (*this)[index];
    }

    const T& at(size_t index) const {
        if (index >= size()) {
            throw std::out_of_range("You're out of range =(");
        }
        return (*this)[index];
    }

    void push_back(const T& obj) {
        new (external_array_[last_block_] + after_last_element_index_) T(obj);
        push_helper(after_last_element_index_, last_block_, BLOCK_SIZE_ - 1,
                    amount_of_blocks_ - 1);
        ++after_last_element_index_;
        last_block_ += after_last_element_index_ / BLOCK_SIZE_;
        after_last_element_index_ %= BLOCK_SIZE_;
    }

    void pop_back() {
        (*this)[size() - 1].~T();
        if (after_last_element_index_ == 0) {
            --last_block_;
            after_last_element_index_ = BLOCK_SIZE_ - 1;
        } else {
            --after_last_element_index_;
        }
    }

    void push_front(const T& obj) {
        size_t prev_first_element_index, prev_first_block;
        if (first_element_index_ == 0) {
            prev_first_block = first_block_ - 1;
            prev_first_element_index = BLOCK_SIZE_ - 1;
        } else {
            prev_first_block = first_block_;
            prev_first_element_index = first_element_index_ - 1;
        }
        new (external_array_[prev_first_block] + prev_first_element_index)
            T(obj);
        push_helper(prev_first_element_index, prev_first_block, 0, 0);
        first_element_index_ = BLOCK_SIZE_ - first_element_index_;
        first_block_ -= (first_element_index_ / BLOCK_SIZE_);
        first_element_index_ %= BLOCK_SIZE_;
        first_element_index_ = BLOCK_SIZE_ - first_element_index_ - 1;
    }

    void pop_front() {
        (*this)[0].~T();
        if (first_element_index_ == BLOCK_SIZE_ - 1) {
            ++first_block_;
            first_element_index_ = 0;
        } else {
            ++first_element_index_;
        }
    }

    void insert(const iterator& it, const T& obj) {
        if (size() == 0 || it == end()) {
            push_back(obj);
        } else {
            T new_obj = *(end() - 1);
            for (auto copy = end() - 1; copy != it; --copy) {
                std::swap(*copy, *(copy - 1));
            }
            *it = obj;
            push_back(new_obj);
        }
    }

    void erase(const iterator& it) {
        for (auto copy = it; copy != end() - 1; ++copy) {
            std::swap(*copy, *(copy + 1));
        }
        pop_back();
    }

  private:
    void swap(Deque<T>& other) {
        std::swap(amount_of_blocks_, other.amount_of_blocks_);
        std::swap(external_array_, other.external_array_);
        std::swap(first_block_, other.first_block_);
        std::swap(first_element_index_, other.first_element_index_);
        std::swap(last_block_, other.last_block_);
        std::swap(after_last_element_index_, other.after_last_element_index_);
    }

    void try_allocate(T** arr, size_t cur_amount_of_blocks) {
        size_t i = 0;
        try {
            for (; i < cur_amount_of_blocks; ++i) {
                arr[i] =
                    reinterpret_cast<T*>(new char[sizeof(T) * BLOCK_SIZE_]);
            }
        } catch (...) {
            for (size_t j = 0; j < i; ++j) {
                delete[] reinterpret_cast<char*>(arr[j]);
            }
            delete[] arr;
            throw;
        }
    }

    void push_helper(size_t cur_index, size_t cur_block,
                     size_t edge_condition_index, size_t edge_condition_block) {
        if (cur_index == edge_condition_index &&
            cur_block == edge_condition_block) {
            size_t new_amount = last_block_ - first_block_ + 1;
            T** new_external_array = new T*[3 * new_amount];
            try {
                try_allocate(new_external_array, 3 * new_amount);
            } catch (...) {
                (external_array_[cur_block] + cur_index)->~T();
                throw;
            }
            for (size_t i = first_block_; i <= last_block_; ++i) {
                std::swap(new_external_array[i - first_block_ + new_amount],
                          external_array_[i]);
            }
            first_block_ = new_amount;
            last_block_ = 2 * new_amount - 1;
            deallocate();
            amount_of_blocks_ = 3 * new_amount;
            external_array_ = new_external_array;
        }
    }

    bool empty() const {
        return size() == 0;
    }

    void destroy(size_t d) {
        for (size_t j = 0; j < d; ++j) {
            pop_back();
        }
    }

    void deallocate() const {
        for (size_t k = 0; k < amount_of_blocks_; ++k) {
            delete[] reinterpret_cast<char*>(external_array_[k]);
        }
        delete[] external_array_;
    }

    static const size_t BLOCK_SIZE_ = 32;
    static const size_t DEFAULT_CAPACITY_ = 8;
    size_t amount_of_blocks_;
    T** external_array_;
    size_t first_block_;
    size_t first_element_index_;
    size_t last_block_;
    size_t after_last_element_index_;
};
