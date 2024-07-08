#include "shared.hpp"
#include <iostream>
#include <mutex>
#include <thread>
 
class ThreadSafeCounter
{
public:
    ThreadSafeCounter() = default;
 
    // Multiple threads/readers can read the counter's value at the same time.
    unsigned int get() const
    {
        sh::shared_lock<sh::recursive_shared_mutex> lock(mutex_);
        return value_;
    }
 
    // Only one thread/writer can increment/write the counter's value.
    void increment()
    {
        std::unique_lock<sh::recursive_shared_mutex> lock(mutex_);
        ++value_;
    }
 
    // Only one thread/writer can reset/write the counter's value.
    void reset()
    {
        std::unique_lock<sh::recursive_shared_mutex> lock(mutex_);
        value_ = 0;
    }
 
private:
    mutable sh::recursive_shared_mutex mutex_;
    unsigned int value_{};
};
 
int main()
{
    ThreadSafeCounter counter;
 
    auto increment_and_print = [&counter]()
    {
        for (int i{}; i != 3; ++i)
        {
            counter.increment();
            std::cout << std::this_thread::get_id() << ' ' << counter.get() << '\n';
        }
    };
 
    std::thread thread1(increment_and_print);
    std::thread thread2(increment_and_print);
 
    thread1.join();
    thread2.join();
}


