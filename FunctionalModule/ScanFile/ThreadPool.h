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
    ThreadPool(size_t threads = 0)
        : stop(false)
    {
        if (threads == 0)   // 设置默认线程数
        {
            SYSTEM_INFO sysInfo;
            GetSystemInfo(&sysInfo);
            threads = sysInfo.dwNumberOfProcessors;
        }
        for (size_t i = 0; i < threads; ++i)
            workers.emplace_back(
                [this]
                {
                    for (;;)
                    {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(queue_mutex);
                            this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });
                            if (this->stop && this->tasks.empty())
                            {
                                return;
                            }
                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }
                        task();
                    }
                });
    }

    template<class F, class... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    {
        using return_type = typename std::result_of<F(Args...)>::type;

        auto task = std::make_shared<std::packaged_task<return_type()>>(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

        std::future<return_type> res = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex);

            if (stop)
            {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }

            tasks.emplace([task]() { (*task)(); });
        }
        condition.notify_one();
        return res;
    }

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
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers)
            worker.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

#endif
