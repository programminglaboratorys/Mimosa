#include <mimosa/mimosa.hpp>

#include <iostream>

#include <mutex>
#include <thread>
#include <utility>

template <typename T>
class optional {
    bool is_valid;
    T ret;
};

template <typename T>
class MutexGuard;

template <typename T>
class Mutex {
public:

    Mutex(T value) : _value(std::move(value)) {}
    Mutex(const Mutex&) = delete; // make it uncopyable

    MutexGuard<T> lock() {
        return MutexGuard<T>(this);
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

    MutexGuard(MutexGuard&& o) noexcept : _m(o._m) {
        o._m = nullptr;
    }

    explicit MutexGuard(Mutex<T>* mutex) {
        _m = mutex;
        if (_m) mutex->mut.lock();
    }

    ~MutexGuard() {
        if (_m) _m->mut.unlock();
    }

    T* operator->() { return &_m->_value; }

    T& operator*() {
        return _m->_value;
    }

    bool is_valid() {
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

int main() {
    namespace mimo = mimosa;


    //man1 leaves the crowd to go to the phone booth
    std::thread man1(makeACallFromPhoneBooth);
    //Although man2 appears to start second, there's a good chance he might
    //reach the phone booth before man1
    std::thread man2(makeACallFromPhoneBooth);
    //And hey, man3 also joined the race to the booth
    std::thread man3(makeACallFromPhoneBooth);
    
    man1.join();//man1 finished his phone call and joins the crowd
    man2.join();//man2 finished his phone call and joins the crowd
    man3.join();//man3 finished his phone call and joins the crowd


    return 0;
}
