#include <sys/resource.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <deque>
#include <fstream>
#include <iostream>
#include <list>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "list.h"
#include "stack_allocator.h"

constexpr size_t STORAGE_SIZE = 200'000'000;
StackStorage<STORAGE_SIZE> STATIC_STORAGE;  // NOLINT

template <typename Alloc = std::allocator<int>>
void BasicListTest(Alloc alloc = Alloc()) {
    List<int, Alloc> lst(alloc);

    assert(lst.size() == 0);

    lst.push_back(3);
    lst.push_back(4);
    lst.push_front(2);
    lst.push_back(5);
    lst.push_front(1);

    std::reverse(lst.begin(), lst.end());
    // now lst is 5 4 3 2 1

    assert(lst.size() == 5);

    std::string s;
    for (int x : lst) {
        s += std::to_string(x);
    }
    assert(s == "54321");
    //std::cerr << " check 1.1 ok, list contains 5 4 3 2 1" << std::endl;

    auto cit = lst.cbegin();
    std::advance(cit, 3);

    lst.insert(cit, 6);
    lst.insert(cit, 7);

    std::advance(cit, -3);
    lst.insert(cit, 8);
    lst.insert(cit, 9);
    // now lst is 5 4 8 9 3 6 7 2 1

    assert(lst.size() == 9);

    s.clear();
    for (int x : lst) {
        s += std::to_string(x);
    }
    assert(s == "548936721");
    //std::cerr << " check 1.2 ok, list contains 5 4 8 9 3 6 7 2 1" << std::endl;

    lst.erase(lst.cbegin());
    lst.erase(cit);

    lst.pop_front();
    lst.pop_back();

    const auto copy = lst;
    assert(lst.size() == 5);
    assert(copy.size() == 5);
    // now both lists are 8 9 6 7 2

    s.clear();
    for (int x : lst) {
        s += std::to_string(x);
    }
    assert(s == "89672");
    //std::cerr << " check 1.3 ok, list contains 8 9 6 7 2" << std::endl;

    auto rit = lst.rbegin();
    ++rit;
    lst.erase(rit.base());
    assert(lst.size() == 4);

    rit = lst.rbegin();
    *rit = 3;

    // now lst: 8 9 6 3, copy: 8 9 6 7 2
    s.clear();
    for (int x : lst) {
        s += std::to_string(x);
    }
    assert(s == "8963");

    assert(copy.size() == 5);

    s.clear();
    for (int x : copy) {
        s += std::to_string(x);
    }
    assert(s == "89672");

    //std::cerr << " check 1.4 ok, list contains 8 9 6 3, another list is still 8 9 6 7 2" << std::endl;

    typename List<int, Alloc>::const_reverse_iterator crit = rit;
    crit = copy.rbegin();
    assert(*crit == 2);

    cit = crit.base();
    std::advance(cit, -2);
    assert(*cit == 7);
}

struct VerySpecialType {
    int x = 0;
    explicit VerySpecialType(int x)
        : x(x) {}
};

struct NotDefaultConstructible {
    NotDefaultConstructible() = delete;
    NotDefaultConstructible(VerySpecialType x)
        : x(x) {}
    VerySpecialType x;
};

struct Accountant {
    // Some field of strange size
    char arr[40];

    static size_t ctor_calls;  // NOLINT
    static size_t dtor_calls;  // NOLINT

    static void reset() {
        ctor_calls = 0;
        dtor_calls = 0;
    }

    Accountant() {
        ++ctor_calls;
    }
    Accountant(const Accountant& /*unused*/) {
        ++ctor_calls;
    }

    Accountant& operator=(const Accountant& rhs) {
        if (this == &rhs) {
            return *this;
        }
        // Actually, when it comes to assign one list to another,
        // list can use element-wise assignment instead of destroying nodes and creating new ones
        ++ctor_calls;
        ++dtor_calls;
        return *this;
    }

    Accountant(Accountant&&) = delete;
    Accountant& operator=(Accountant&&) = delete;

    ~Accountant() {
        ++dtor_calls;
    }
};

size_t Accountant::ctor_calls = 0;  // NOLINT
size_t Accountant::dtor_calls = 0;  // NOLINT

template <typename Alloc = std::allocator<NotDefaultConstructible>>
void TestNotDefaultConstructible(Alloc alloc = Alloc()) {
    List<NotDefaultConstructible, Alloc> lst(alloc);
    assert(lst.size() == 0);
    lst.push_back(VerySpecialType(0));
    assert(lst.size() == 1);
    lst.pop_front();
    assert(lst.size() == 0);
}

template <typename Alloc = std::allocator<Accountant>>
void TestAccountant(Alloc alloc = Alloc()) {
    Accountant::reset();

    {
        List<Accountant, Alloc> lst(5, alloc);
        assert(lst.size() == 5);
        assert(Accountant::ctor_calls == 5);

        List<Accountant, Alloc> another = lst;
        assert(another.size() == 5);
        assert(Accountant::ctor_calls == 10);
        assert(Accountant::dtor_calls == 0);

        another.pop_back();
        another.pop_front();
        assert(Accountant::dtor_calls == 2);

        lst = another;  // dtor_calls += 5, ctor_calls += 3
        assert(another.size() == 3);
        assert(lst.size() == 3);

        assert(Accountant::ctor_calls == 13);
        assert(Accountant::dtor_calls == 7);

    }  // dtor_calls += 6

    assert(Accountant::ctor_calls == 13);
    assert(Accountant::dtor_calls == 13);
}

struct ThrowingAccountant : public Accountant {
    static bool need_throw;  // NOLINT

    int value = 0;

    ThrowingAccountant(int value = 0)
        : Accountant(), value(value) {
        if (need_throw && ctor_calls % 5 == 4) {
            throw std::string("Ahahahaha you have been cocknut");
        }
    }

    ThrowingAccountant(const ThrowingAccountant& other)
        : Accountant(other), value(other.value) {
        if (need_throw && ctor_calls % 5 == 4) {
            throw std::string("Ahahahaha you have been cocknut");
        }
    }

    ThrowingAccountant& operator=(const ThrowingAccountant& other) {
        value = other.value;
        ++ctor_calls;
        ++dtor_calls;
        if (need_throw && ctor_calls % 5 == 4) {
            throw std::string("Ahahahaha you have been cocknut");
        }
        return *this;
    }
};

bool ThrowingAccountant::need_throw = false;  // NOLINT

void TestExceptionSafety() {
    Accountant::reset();

    ThrowingAccountant::need_throw = true;

    try {
        List<ThrowingAccountant> lst(8);
    } catch (...) {
        assert(Accountant::ctor_calls == 4);
        assert(Accountant::dtor_calls == 4);
    }

    ThrowingAccountant::need_throw = false;
    List<ThrowingAccountant> lst(8);

    List<ThrowingAccountant> lst2;
    for (int i = 0; i < 13; ++i) {
        lst2.push_back(i);
    }

    Accountant::reset();
    ThrowingAccountant::need_throw = true;

    try {
        auto lst3 = lst2;
    } catch (...) {
        assert(Accountant::ctor_calls == 4);
        assert(Accountant::dtor_calls == 4);
    }

    Accountant::reset();

    try {
        lst = lst2;
    } catch (...) {
        assert(Accountant::ctor_calls == 4);
        assert(Accountant::dtor_calls == 4);

        // Actually it may not be 8 (although de facto it is), but the only thing we can demand here
        // is the abscence of memory leaks
        //
        //assert(lst.size() == 8);
    }
}

void TestAlignment() {

    StackStorage<200'000> storage;

    StackAllocator<char, 200'000> charalloc(storage);

    StackAllocator<int, 200'000> intalloc(charalloc);

    auto* pchar = charalloc.allocate(3);

    auto* pint = intalloc.allocate(1);

    assert((void*)pchar != (void*)pint);

    assert(reinterpret_cast<uintptr_t>(pint) % sizeof(int) == 0);

    charalloc.deallocate(pchar, 3);

    pchar = charalloc.allocate(555);

    intalloc.deallocate(pint, 1);

    StackAllocator<long double, 200'000> ldalloc(charalloc);

    auto* pld = ldalloc.allocate(25);

    assert(reinterpret_cast<uintptr_t>(pld) % sizeof(long double) == 0);

    charalloc.deallocate(pchar, 555);
    ldalloc.deallocate(pld, 25);
}

template <typename T, bool PropagateOnConstruct, bool PropagateOnAssign>
struct WhimsicalAllocator : public std::allocator<T> {
    std::shared_ptr<int> number;

    auto select_on_container_copy_construction() const {
        return PropagateOnConstruct
                   ? WhimsicalAllocator<T, PropagateOnConstruct, PropagateOnAssign>()
                   : *this;
    }

    struct propagate_on_container_copy_assignment
        : std::conditional_t<PropagateOnAssign, std::true_type,
                             std::false_type> {};

    template <typename U>
    struct rebind {
        using other =
            WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>;
    };

    WhimsicalAllocator()
        : number(std::make_shared<int>(counter)) {
        ++counter;
    }

    template <typename U>
    WhimsicalAllocator(const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another)
        : number(another.number) {}

    template <typename U>
    auto& operator=(const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another) {
        number = another.number;
        return *this;
    }

    template <typename U>
    bool operator==(
        const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another) const {
        return std::is_same_v<decltype(*this), decltype(another)> &&
               *number == *another.number;
    }

    template <typename U>
    bool operator!=(
        const WhimsicalAllocator<U, PropagateOnConstruct, PropagateOnAssign>& another) const {
        return !(*this == another);
    }

    static size_t counter;  // NOLINT
};

template <typename T, bool PropagateOnConstruct, bool PropagateOnAssign>
size_t WhimsicalAllocator<T, PropagateOnConstruct, PropagateOnAssign>::counter = 0;  // NOLINT

void TestWhimsicalAllocator() {
    {
        List<int, WhimsicalAllocator<int, true, true>> lst;

        lst.push_back(1);
        lst.push_back(2);

        auto copy = lst;
        assert(copy.get_allocator() != lst.get_allocator());

        lst = copy;
        assert(copy.get_allocator() == lst.get_allocator());
    }
    {
        List<int, WhimsicalAllocator<int, false, false>> lst;

        lst.push_back(1);
        lst.push_back(2);

        auto copy = lst;
        assert(copy.get_allocator() == lst.get_allocator());

        lst = copy;
        assert(copy.get_allocator() == lst.get_allocator());
    }
    {
        List<int, WhimsicalAllocator<int, true, false>> lst;

        lst.push_back(1);
        lst.push_back(2);

        auto copy = lst;
        assert(copy.get_allocator() != lst.get_allocator());

        lst = copy;
        assert(copy.get_allocator() != lst.get_allocator());
    }
}

template <class List>
int ListPerformanceTest(List&& l) {
    using std::chrono::high_resolution_clock;
    using std::chrono::milliseconds;

    std::ostringstream oss;

    auto start = high_resolution_clock::now();

    for (int i = 0; i < 1'000'000; ++i) {
        l.push_back(i);
    }
    auto it = l.begin();
    for (int i = 0; i < 1'000'000; ++i) {
        l.push_front(i);
    }
    oss << *it;

    auto it2 = std::prev(it);
    for (int i = 0; i < 2'000'000; ++i) {
        l.insert(it, i);
        if (i % 534'555 == 0) {
            oss << *it;
        }
    }
    oss << *it;

    for (int i = 0; i < 1'500'000; ++i) {
        l.pop_back();
        if (i % 342'985 == 0) {
            oss << *l.rbegin();
        }
    }
    oss << *l.rbegin();

    for (int i = 0; i < 1'000'000; ++i) {
        l.erase(it2++);
        if (i % 432'098 == 0) {
            oss << *it2;
        }
    }
    oss << *it2;

    for (int i = 0; i < 1'000'000; ++i) {
        l.pop_front();
    }
    oss << *l.begin();

    for (int i = 0; i < 1'000'000; ++i) {
        l.push_back(i);
    }
    oss << *l.rbegin();

    assert(oss.str() ==
           "0000009999986570133140281971043162805814999990432098864196999999100"
           "0000999999");

    auto finish = high_resolution_clock::now();
    return duration_cast<milliseconds>(finish - start).count();
}

template <typename Alloc>
void DequeTest() {
    Alloc alloc(STATIC_STORAGE);

    std::deque<char, Alloc> d(alloc);

    d.push_back(1);
    assert(d.back() == 1);

    d.resize(2'500'000, 5);
    assert(d[1'000'000] == 5);

    d.pop_back();
    for (int i = 0; i < 2'000'000; ++i) {
        d.push_back(i % 100);
    }

    assert(d.size() == 4'499'999);
    assert(d[4'000'000] == 1);

    for (int i = 0; i < 4'000'000; ++i) {
        d.pop_front();
    }

    assert(d[400'000] == 1);
}

int main() {

    const rlim_t kStackSize = 210 * 1024 * 1024;  // min stack size = 16 MB
    struct rlimit rl;
    int result;

    result = getrlimit(RLIMIT_STACK, &rl);
    if (result != 0) {
        std::cerr << "Failed to get current stack size\n";
        abort();
    }

    std::cerr << rl.rlim_cur << '\n';

    if (rl.rlim_cur < kStackSize) {
        rl.rlim_cur = kStackSize;
        result = setrlimit(RLIMIT_STACK, &rl);
        if (result != 0) {
            std::cerr << "Failed to set bigger stack size\n";
            abort();
        }
        std::cerr << "Stack size is successfully set to " << kStackSize << '\n';
    }

    static_assert(!std::is_assignable_v<decltype(*List<int>().cbegin()), int>);
    static_assert(
        !std::is_assignable_v<List<int>::iterator, List<int>::const_iterator>);

    static_assert(std::is_same_v<
                  std::iterator_traits<List<int>::iterator>::iterator_category,
                  std::bidirectional_iterator_tag>);

    BasicListTest<>();

    std::cerr << "Test 1 (BasicTest) with std::allocator passed. Now will "
                 "repeat with StackAllocator"
              << std::endl;

    {
        StackStorage<200'000> storage;
        StackAllocator<int, 200'000> alloc(storage);

        BasicListTest<StackAllocator<int, 200'000>>(alloc);
    }

    std::cerr << "Test 1 with StackAllocator passed." << std::endl;

    TestAccountant<>();

    std::cerr << "Test 2 (counting) with std::allocator passed. Now will "
                 "repeat with StackAllocator"
              << std::endl;

    {
        StackStorage<200'000> storage;
        StackAllocator<int, 200'000> alloc(storage);

        TestAccountant<StackAllocator<Accountant, 200'000>>(alloc);
    }

    std::cerr << "Test 2 with StackAllocator passed." << std::endl;

    TestExceptionSafety();

    std::cerr << "Test 3 (ExceptionSafety) passed." << std::endl;

    TestAlignment();

    std::cerr << "Test 4 (Alignment) passed." << std::endl;

    TestNotDefaultConstructible<>();

    {
        StackStorage<200'000> storage;
        StackAllocator<int, 200'000> alloc(storage);

        TestNotDefaultConstructible<
            StackAllocator<NotDefaultConstructible, 200'000>>(alloc);
    }

    std::cerr << "Test 5 (NotDefaultConstructible) passed." << std::endl;

    DequeTest<StackAllocator<char, STORAGE_SIZE>>();

    std::cerr << "Test 6 (Deque with StackAllocator) passed." << std::endl;

    TestWhimsicalAllocator();

    std::cerr << "Test 7 (Allocator Awareness) passed." << std::endl;

    std::cerr << "Starting performance test. First, let's test performance of "
                 "different allocators with std::list."
              << std::endl;

    std::cerr << "Tests passed, my sweetheart!" << std::endl;

    if (std::is_assignable_v<List<int>, std::list<int>> ||
        std::is_assignable_v<std::list<int>, List<int>>) {
        std::cerr << "....but you must use your own List, not std::list!"
                  << std::endl;
        throw std::runtime_error("Bad guy!");
    }

    std::cout << 0;
}
