template<typename T>
class WeakPtr;

template<typename T>
class SharedPtr {
  struct BaseControlBlock {
    size_t shared_count{0};
    size_t weak_count{0};
    virtual void useDeleter() = 0;
    virtual void useDeallocator() = 0;
    virtual T *pointer() = 0;
    virtual ~BaseControlBlock() = default;
  };

  template<typename U, typename Deleter = std::default_delete<U>, typename Alloc = std::allocator<U>>
  struct ControlBlockRegular : BaseControlBlock {
    U *object;
    Deleter deleter;
    Alloc alloc;

    ControlBlockRegular(U *object, const Deleter &deleter, const Alloc &alloc)
        : object(object), deleter(deleter), alloc(alloc) {}

    void useDeleter() override {
      deleter(object);
    }

    void useDeallocator() override {
      using ControlBlockRegularAlloc = typename std::allocator_traits<Alloc>::template rebind_traits<ControlBlockRegular>;
      typename ControlBlockRegularAlloc::allocator_type newAlloc = alloc;
      newAlloc.deallocate(this, 1);
    }

    U *pointer() override {
      return object;
    }

    ~ControlBlockRegular() = default;
  };

  template<typename U, typename Alloc = std::allocator<U>>
  struct ControlBlockMakeShared : BaseControlBlock {
    U object;
    Alloc alloc;

    template<typename... Args>
    ControlBlockMakeShared(Alloc alloc, Args &&... args)
        : object(std::forward<Args>(args)...), alloc(alloc) {}

    T *pointer() override {
      return &object;
    }

    void useDeleter() override {
      std::allocator_traits<Alloc>::destroy(alloc, &object);
    }

    void useDeallocator() override {
      using ControlBlockMakeSharedAlloc = typename std::allocator_traits<Alloc>::template rebind_traits<
          ControlBlockMakeShared>;
      typename ControlBlockMakeSharedAlloc::allocator_type newAlloc = alloc;
      newAlloc.deallocate(this, 1);
    }

    ~ControlBlockMakeShared() = default;
  };

  template<typename U>
  friend
  class SharedPtr;

  template<typename U>
  friend
  class WeakPtr;

  template<typename U, typename Alloc, typename... Args>
  friend SharedPtr<U> allocateShared(const Alloc &, Args &&...);

  BaseControlBlock *cb;

 public:
  SharedPtr() = default;

  ~SharedPtr() {
    if (cb == nullptr) {
      return;
    }
    --cb->shared_count;
    if (cb->shared_count == 0) {
      cb->useDeleter();
      if (cb->weak_count == 0) {
        cb->useDeallocator();
      }
    }
  }

  template<typename U, typename Deleter = std::default_delete<T>, typename Alloc = std::allocator<T>>
  SharedPtr(U *ptr, const Deleter &deleter = Deleter(), const Alloc &alloc = Alloc()) {
    typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular<U, Deleter, Alloc>>
        newAlloc = alloc;
    auto newCB =
        std::allocator_traits<typename std::allocator_traits<Alloc>::template rebind_alloc<ControlBlockRegular<U,
                                                                                                               Deleter,
                                                                                                               Alloc>>>::allocate(
            newAlloc,
            1);
    cb = newCB;
    new(newCB) ControlBlockRegular<U, Deleter, Alloc>(ptr, deleter, alloc);
    ++cb->shared_count;
  }

  template<typename U>
  SharedPtr(const WeakPtr<U> &wp) : cb(reinterpret_cast<BaseControlBlock *>(wp.cb)) {
    if (cb != nullptr) {
      ++cb->shared_count;
    }
  }

  SharedPtr(const SharedPtr &sp) : cb(reinterpret_cast<BaseControlBlock *>(sp.cb)) {
    if (cb != nullptr) {
      ++cb->shared_count;
    }
  }

  template<typename U>
  SharedPtr(const SharedPtr<U> &sp): cb(reinterpret_cast<BaseControlBlock *>(sp.cb)) {
    if (cb != nullptr) {
      ++cb->shared_count;
    }
  }

  SharedPtr(SharedPtr &&sp) : cb(reinterpret_cast<BaseControlBlock *>(sp.cb)) {
    sp.cb = nullptr;
  }

  template<typename U>
  SharedPtr(SharedPtr<U> &&sp): cb(reinterpret_cast<BaseControlBlock *>(sp.cb)) {
    sp.cb = nullptr;
  }

  void swap(SharedPtr &sp) {
    std::swap(cb, sp.cb);
  }

  SharedPtr &operator=(const SharedPtr &sp) {
    SharedPtr temp(sp);
    temp.swap(*this);
    return *this;
  }

  template<typename U>
  SharedPtr &operator=(const SharedPtr<U> &sp) {
    SharedPtr temp(sp);
    temp.swap(*this);
    return *this;
  }

  SharedPtr &operator=(SharedPtr &&sp) {
    SharedPtr temp(std::move(sp));
    temp.swap(*this);
    return *this;
  }

  template<typename U>
  SharedPtr &operator=(SharedPtr<U> &&sp) {
    SharedPtr temp(std::move(sp));
    temp.swap(*this);
    return *this;
  }

  size_t use_count() const {
    return cb->shared_count;
  }

  void reset() {
    SharedPtr<T>().swap(*this);
  }

  template<typename U>
  void reset(U *ptr) {
    SharedPtr<U>(ptr).swap(*this);
  }

  T *get() const {
    return cb ? cb->pointer() : nullptr;
  }

  T *operator->() const {
    return &**this;
  }

  T &operator*() const {
    return *get();
  }
};

template<typename T, typename Alloc, typename... Args>
SharedPtr<T> allocateShared(const Alloc &alloc, Args &&... args) {
  using ControlBlockMakeSharedAlloc = typename std::allocator_traits<Alloc>::template rebind_alloc<typename SharedPtr<T>::template ControlBlockMakeShared<
      T,
      Alloc>>;
  ControlBlockMakeSharedAlloc newAlloc = alloc;
  auto newCB = std::allocator_traits<ControlBlockMakeSharedAlloc>::allocate(newAlloc, 1);
  std::allocator_traits<ControlBlockMakeSharedAlloc>::construct(newAlloc, newCB, alloc, std::forward<Args>(args)...);
  SharedPtr<T> allocated;
  allocated.cb = newCB;
  ++allocated.cb->shared_count;
  return allocated;
}

template<typename U, typename... Args>
SharedPtr<U> makeShared(Args &&... args) {
  return allocateShared<U>(std::allocator<U>(), std::forward<Args>(args)...);
}

template<typename T>
class WeakPtr {
  template<typename U>
  friend
  class SharedPtr;

  template<typename U>
  friend
  class WeakPtr;

  using BaseControlBlock = typename SharedPtr<T>::BaseControlBlock;

  BaseControlBlock *cb{nullptr};

 public:
  WeakPtr() = default;

  ~WeakPtr() {
    if (cb == nullptr) {
      return;
    }
    --cb->weak_count;
    if (cb->weak_count == 0 && cb->shared_count == 0) {
      cb->useDeallocator();
    }
  }

  template<typename U>
  WeakPtr(const SharedPtr<U> &sp) : cb(reinterpret_cast<BaseControlBlock *>(sp.cb)) {
    if (cb != nullptr) {
      ++cb->weak_count;
    }
  }

  WeakPtr(const WeakPtr &wp) : cb(reinterpret_cast<BaseControlBlock *>(wp.cb)) {
    if (cb != nullptr) {
      ++cb->weak_count;
    }
  }

  template<typename U>
  WeakPtr(const WeakPtr<U> &wp) : cb(reinterpret_cast<BaseControlBlock *>(wp.cb)) {
    if (cb != nullptr) {
      ++cb->weak_count;
    }
  }

  WeakPtr(WeakPtr &&wp) : cb(wp.cb) {
    wp.cb = nullptr;
  }

  template<typename U>
  WeakPtr(WeakPtr<U> &&wp) : cb(wp.cb) {
    wp.cb = nullptr;
  }

  void swap(WeakPtr<T> &wp) {
    std::swap(cb, wp.cb);
  }

  WeakPtr &operator=(const WeakPtr &wp) {
    WeakPtr temp(wp);
    temp.swap(*this);
    return *this;
  }

  template<typename U>
  WeakPtr &operator=(const WeakPtr<U> &wp) {
    WeakPtr temp = wp;
    temp.swap(*this);
    return *this;
  }

  WeakPtr &operator=(WeakPtr &&wp) {
    WeakPtr temp(std::move(wp));
    temp.swap(*this);
    return *this;
  }

  template<typename U>
  WeakPtr &operator=(WeakPtr<U> &&wp) {
    WeakPtr temp(std::move(wp));
    temp.swap(*this);
    return *this;
  }

  size_t use_count() const {
    return cb->shared_count;
  }

  bool expired() const {
    return cb && cb->shared_count == 0;
  }

  SharedPtr<T> lock() const {
    return expired() ? SharedPtr<T>() : SharedPtr<T>(*this);
  }
};
