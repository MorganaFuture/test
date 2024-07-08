
#include <condition_variable>
#include <unordered_map>
#include <mutex>
#include <thread>

namespace sh {

class recursive_shared_mutex {
public:
    recursive_shared_mutex() = default;
    recursive_shared_mutex(const recursive_shared_mutex&) = delete;
    recursive_shared_mutex& operator=(const recursive_shared_mutex&) = delete;
    ~recursive_shared_mutex() = default;
    
    void lock();
    bool try_lock();
    void unlock();
    void lock_shared();
    bool try_lock_shared();
    void unlock_shared();

private:
    std::recursive_mutex iMutex;
    std::condition_variable_any iExclusiveRelease;
    std::condition_variable_any iSharedRelease;
    std::unordered_map<std::thread::id, int> iRecursiveLockCounts;
    bool iExclusiveLocked = false;
    int iNumReaders = 0;
    std::thread::id iWriteThread;
};

template<class Mutex>
class shared_lock {
public:
    explicit shared_lock(Mutex& m) : mutex_(m) {
        mutex_.lock_shared();
        locked_ = true;
    }

    ~shared_lock() {
        if (locked_) {
            mutex_.unlock_shared();
        }
    }

    shared_lock(const shared_lock&) = delete;
    shared_lock& operator=(const shared_lock&) = delete;

private:
    Mutex& mutex_;
    bool locked_ = false;
};

} // namespace sh


