#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <windows.h>

// 线程池
class ThreadPool
{
public:
    /**
     * @brief 线程池构造函数
     *
     * 初始化线程池对象，并创建指定数量的工作线程。
     *
     * @param threads 线程池中工作线程的数量，默认为0。如果为0，则默认线程数等于CPU核心数。
     */
    ThreadPool(size_t threads = 0)
        : stop(false)
    {
        if (threads == 0)   // 设置默认线程数
        {
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            threads = sysInfo.dwNumberOfProcessors; // 默认线程数等于CPU核心数
        }
        // 创建工作线程并启动它们
        for (size_t i = 0; i < threads; ++i)
        {
            workers.emplace_back(std::bind(&ThreadPool::workFun, this));
        }
            
    }

    // 添加任务到线程池
    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;
        // 创建一个std::packaged_task对象，它将任务封装在一个可执行的函数中。
        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future(); // 创建任务并获取其未来结果
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            // 如果线程池已经停止，则抛出异常
            if (stop)
            {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks.emplace([task]() { (*task)(); }); // 将任务封装到std::function中，并推入队列中
        }
        condition.notify_one(); // 唤醒一个等待的线程
        return res; // 返回任务的未来结果

    }

    /**
     * @brief 退出函数，清空任务队列
     *
     * 该函数用于在程序退出时清空任务队列，确保所有任务都被处理完毕。
     * 使用std::unique_lock锁定队列互斥锁，以确保线程安全。
     * 在锁定期间，循环检查任务队列是否为空，如果不为空，则弹出队列中的任务。
     */
    void quit()
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        while (!tasks.empty())
        {
            tasks.pop();
        }
    }

    ~ThreadPool()
    {
        {
            // 标记线程池为停止状态，并唤醒所有等待的线程
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();  // 唤醒所有等待的线程
        // 等待所有工作线程完成，然后销毁它们
        for (std::thread& worker : workers)
        {
            worker.join();
        }
    }

private:
    /**
     * @brief 工作函数，用于从任务队列中取出任务并执行
     *
     * 该函数在一个无限循环中，不断从任务队列中取出任务并执行。如果任务队列为空且已停止工作，则退出循环。
     */
    void workFun()
    {
        std::function<void()> task; // 任务容器
        while (true)
        {
            {
                std::unique_lock<std::mutex> lock(queue_mutex);
                this->condition.wait(lock, [this] 
                {
                    // 如果线程池已停止且任务队列为空，则返回true以退出循环
                    return (this->stop || !this->tasks.empty()); 
                });
                // 如果线程池已停止且任务队列为空，则退出循环
                if (this->stop && this->tasks.empty())
                {
                    return;
                }
                task = std::move(this->tasks.front());  // 从队列中取出任务
                this->tasks.pop();  // 从队列中移除已执行的任务
            }
            task();  // 执行任务
        }
    }

private:
    std::vector<std::thread> workers;  // 线程池中的工作线程
    std::queue<std::function<void()>> tasks;  // 任务队列

    std::mutex queue_mutex;
    std::condition_variable condition;  // 条件变量，用于阻塞和唤醒线程
    bool stop;  // 线程池是否停止的标志
};

#endif
