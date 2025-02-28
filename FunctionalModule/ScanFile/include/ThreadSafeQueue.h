#ifndef THREADSAFEQUEUE_H
#define THREADSAFEQUEUE_H

#include <queue>
#include "SpinLock.h"
#include <mutex>
#include <stdexcept>

// 线程安全的队列实现，使用自旋锁代替互斥锁来提高性能。
template<typename T>
class ThreadSafeQueue {
public:
    // 入队操作
    void enqueue(const T& item) {
        std::lock_guard<SpinLock> lock(m_lock);
        m_q.push(item);
    }

    // 获取队列头部元素，但不移除它
    T front() {
        std::lock_guard<SpinLock> lock(m_lock);
        if (m_q.empty()) {
            throw std::runtime_error("Queue is empty"); // 抛出异常，表示队列为空
        }
        auto v = m_q.front();
        return v;
    }

    // 出队操作，但不返回元素值，仅移除队列头部元素
    void pop()  {
        std::lock_guard<SpinLock> lock(m_lock);
        if (!m_q.empty()) 
        {
            m_q.pop();
        }
    }

    // 判断队列是否为空
    bool empty()  {
        std::lock_guard<SpinLock> lock(m_lock);
        return m_q.empty();
    }

    // 获取队列大小
    size_t size()  {
        std::lock_guard<SpinLock> lock(m_lock);
        return m_q.size();
    }

    // 清空队列
    void clear()  {
        std::lock_guard<SpinLock> lock(m_lock);
        while (!m_q.empty()) {
            m_q.pop();
        }
    }
private:
    std::queue<T> m_q;
    SpinLock m_lock; // 使用自旋锁代替互斥锁，提高性能
};
#endif