#pragma once

#include <cstddef>

template <size_t N>
class alignas(::max_align_t) StackStorage {
  public:
    StackStorage() = default;

    StackStorage(const StackStorage&) = delete;

    StackStorage operator=(const StackStorage&) = delete;

    char* allocate(size_t count, size_t alignment) {
        shift_ += count;
        shift_ += (alignment - shift_ % alignment) % alignment;
        return stack_array_ + shift_ - count;
    }

  private:
    char stack_array_[N];
    size_t shift_{0};
};

template <typename T, size_t N>
class StackAllocator {
  public:
    using value_type = T;

    StackAllocator() = default;

    StackAllocator(StackStorage<N>& stack_storage)
        : p_stack_storage_(&stack_storage) {}

    template <typename U, size_t M>
    friend class StackAllocator;

    template <typename U>
    StackAllocator(const StackAllocator<U, N>& stack_allocator)
        : p_stack_storage_(stack_allocator.p_stack_storage_) {}

    T* allocate(size_t count) {
        return reinterpret_cast<T*>(
            p_stack_storage_->allocate(count * sizeof(T), alignof(T)));
    }
    void deallocate(T* ptr, size_t count) {
        std::ignore = ptr;
        std::ignore = count;
    }

    template <typename U>
    struct rebind {
        using other = StackAllocator<U, N>;
    };

  private:
    StackStorage<N>* p_stack_storage_;
};
