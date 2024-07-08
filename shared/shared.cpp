#include "shared.hpp"

namespace sh {
    
void recursive_shared_mutex::lock() {
    std::unique_lock<std::recursive_mutex> lk(iMutex);
    if (iExclusiveLocked && iWriteThread != std::this_thread::get_id()) {
        iExclusiveRelease.wait(lk, [this]{ return !iExclusiveLocked; });
    }
    iExclusiveLocked = true;
    iWriteThread = std::this_thread::get_id();
    iRecursiveLockCounts[std::this_thread::get_id()]++;
}

bool recursive_shared_mutex::try_lock() {
    std::unique_lock<std::recursive_mutex> lk(iMutex, std::try_to_lock);
    if (lk.owns_lock() && !iExclusiveLocked) {
        iExclusiveLocked = true;
        iWriteThread = std::this_thread::get_id();
        iRecursiveLockCounts[std::this_thread::get_id()]++;
        return true;
    }
    return false;
}

void recursive_shared_mutex::unlock() {
    std::lock_guard<std::recursive_mutex> lg(iMutex);
    if (!iExclusiveLocked || iWriteThread != std::this_thread::get_id()) {
        throw std::logic_error("Unlocking an unlocked mutex or unlocking by a non-owner thread");
    }
    if (iRecursiveLockCounts[std::this_thread::get_id()] > 1) {
        iRecursiveLockCounts[std::this_thread::get_id()]--;
    } else {
        iExclusiveLocked = false;
        iExclusiveRelease.notify_one();
        iRecursiveLockCounts.erase(std::this_thread::get_id());
    }
}

void recursive_shared_mutex::lock_shared() {
    std::unique_lock<std::recursive_mutex> lk(iMutex);
    iSharedRelease.wait(lk, [this]{ return !iExclusiveLocked; });
    iNumReaders++;
}

bool recursive_shared_mutex::try_lock_shared() {
    std::unique_lock<std::recursive_mutex> lk(iMutex, std::try_to_lock);
    if (lk.owns_lock() && !iExclusiveLocked) {
        iNumReaders++;
        return true;
    }
    return false;
}

void recursive_shared_mutex::unlock_shared() {
    std::lock_guard<std::recursive_mutex> lg(iMutex);
    if (iNumReaders == 0) {
        throw std::logic_error("Unlocking an unlocked shared lock");
    }
    iNumReaders--;
    if (iNumReaders == 0) {
        iSharedRelease.notify_one();
    }
}

} // namespace sh