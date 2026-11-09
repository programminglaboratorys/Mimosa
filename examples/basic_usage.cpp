#include <mimosa/mimosa.hpp>

#include <iostream>

#include <mutex>
#include <new>
#include <thread>
#include <type_traits>
#include <functional>
#include <utility>

#define STDX_HAS_STD_OPTIONAL 0

#if (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || (__cplusplus >= 201703L)

#if defined(__has_include)
#if __has_include(<optional>)
#include <optional>
#undef STDX_HAS_STD_OPTIONAL
#define STDX_HAS_STD_OPTIONAL 1
#endif
#endif

#endif

struct adopt_lock_t {};

constexpr adopt_lock_t adopt_lock{};

namespace detail {

#if STDX_HAS_STD_OPTIONAL
    template <typename T>
    using optional = std::optional<T>;

#else
    template <typename T>
    class optional {
        typename std::aligned_storage<sizeof(T), alignof(T)>::type storage;
        bool has_value = false;

        T* ptr() { return reinterpret_cast<T*>(&storage); }
        const T* ptr() const { return reinterpret_cast<const T*>(&storage); }

        void reset() {
            if (has_value) {
                ptr()->~T();
                has_value = false;
            }
        }

    public:
        optional() = default;

        optional(const T& val) : has_value(true) {
            ::new (&storage) T(val);
        }

        optional(T&& val) : has_value(true) {
            ::new (&storage) T(std::move(val));
        }

        ~optional() { reset(); }

        optional(const optional&) = delete;
        optional& operator=(const optional&) = delete;

        optional(optional&& o) noexcept : has_value(o.has_value) {
            if (o.has_value) {
                ::new (&storage) T(std::move(*o.ptr()));
                o.reset();
            }
        }

        optional& operator=(optional&& o) noexcept {
            if (this != &o) {
                reset();
                if (o.has_value) {
                    ::new (&storage) T(std::move(*o.ptr()));
                    has_value = true;
                    o.reset();
                }
            }
            return *this;
        }

        explicit operator bool() const { return has_value; }

        T& operator*() { return *ptr(); }
        const T& operator*() const { return *ptr(); }
        T* operator->() { return ptr(); }
        const T* operator->() const { return ptr(); }
    };
#endif

} // namespace detail

template <typename T>
class MutexGuard;

template <typename T>
class Mutex {
public:

    explicit Mutex(T value) : _value(std::move(value)) {}
    Mutex(const Mutex&) = delete; // make it uncopyable

    MutexGuard<T> lock() {
        return MutexGuard<T>(this);
    }

    detail::optional<MutexGuard<T>> try_lock() { // uses a custom optional if C++11
        if (!mut.try_lock()) return {};
        return MutexGuard<T>(this, adopt_lock);
    }

    // runs callback on the locked data and returns its result BY VALUE
    // a returned reference is decayed into a copy (made while still locked)
    // NOTE: returned pointers are NOT protected. Don't return pointers into the data <3
    template <typename F>
    auto with_lock(F callback) -> typename std::decay<decltype(callback(std::declval<T&>()))>::type {
        auto guard = lock();
        return callback(*guard);
    }
    
private:
    template <typename> friend class MutexGuard;
    T _value;
    std::mutex mut;
};


template <typename T>
class MutexGuard {
public:

    MutexGuard(const MutexGuard&) = delete; // make it uncopyable
    MutexGuard& operator=(const MutexGuard&) = delete;

    //move constructor
    MutexGuard(MutexGuard&& o) noexcept : _m(o._m) {
        o._m = nullptr;
    }

    explicit MutexGuard(Mutex<T>* mutex) {
        _m = mutex;
        mutex->mut.lock();
    }

    explicit MutexGuard(Mutex<T>* mutex, adopt_lock_t) { // used by try_lock
        _m = mutex;
    }

    ~MutexGuard() {
        if (_m) _m->mut.unlock();
    }

    T* operator->() const { return &_m->_value; }

    T& operator*() const {
        return _m->_value;
    }

    bool is_valid() const {
        return _m != nullptr;
    }

private:
    friend Mutex<T>;
    Mutex<T>* _m = nullptr;
};

Mutex<int> m(0);

void makeACallFromPhoneBooth()
{
    auto guard = m.lock();//man gets a hold of the phone booth door and locks it. The other men wait outside
    //man happily talks to his wife from now....
    std::cout << *guard << " Hello Wife" << std::endl;
    (*guard)++;//no other thread can access variable i until m.unlock() is called
    //...until now, with no interruption from other men
    //m.unlock();//man lets go of the door handle and unlocks the door
}
//
//int main() {
//    namespace mimo = mimosa;
//
//
//    //man1 leaves the crowd to go to the phone booth
//    std::thread man1(makeACallFromPhoneBooth);
//    //Although man2 appears to start second, there's a good chance he might
//    //reach the phone booth before man1
//    std::thread man2(makeACallFromPhoneBooth);
//    //And hey, man3 also joined the race to the booth
//    std::thread man3(makeACallFromPhoneBooth);
//    
//    man1.join();//man1 finished his phone call and joins the crowd
//    man2.join();//man2 finished his phone call and joins the crowd
//    man3.join();//man3 finished his phone call and joins the crowd
//
//
//    return 0;
//}


#include <chrono>   // add near the other includes

void makeACallWithTryLock(int id)
{
    while (true)
    {
        auto g = m.try_lock();   // door handle: try it, never wait
        if (g)
        {
            std::cout << "man" << id << " got the booth, counter = " << **g << "\n";
            (**g)++;
            // long call, so the others will find the door busy
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            return;              // guard dies here, door unlocks
        }
        std::cout << "man" << id << " found it busy, waiting outside" << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main() {
    std::thread man1(makeACallWithTryLock, 1);
    std::thread man2(makeACallWithTryLock, 2);
    std::thread man3(makeACallWithTryLock, 3);

    man1.join();
    man2.join();
    man3.join();

    // unlock count check: everyone is done, so this must not hang
    auto g = m.lock();
    std::cout << "final = " << *g << std::endl;   // expect 3

    return 0;
}