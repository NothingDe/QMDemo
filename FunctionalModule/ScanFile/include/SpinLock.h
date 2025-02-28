#ifndef SPINLOCK_H
#define SPINLOCK_H

/**
 * 定义一个简单的自旋锁类 SpinLock，使用 C++11 的原子操作。
 */
#include <atomic>
#include <thread>

class SpinLock {
public:
    SpinLock() 
    {
        flag.clear();
    }

    void lock() {
        while (flag.test_and_set(std::memory_order_acquire));
    }

    void unlock() {
        flag.clear(std::memory_order_release);
    }

private:
    std::atomic_flag flag;
};

#endif // SPINLOCK_H