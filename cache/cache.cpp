#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <chrono>
#include <future>
#include <thread>

class IDatabase {
public:
    virtual bool begin_transaction() = 0;
    virtual bool commit_transaction() = 0;
    virtual bool abort_transaction() = 0;
    virtual std::string get(const std::string& key) = 0;
    virtual std::string set(const std::string& key, const std::string& data) = 0;
    virtual std::string remove(const std::string& key) = 0;
    virtual std::map<std::string, std::string> GetCacheData() = 0;
    virtual ~IDatabase() {}
};

struct Cache
{
public:
    Cache(IDatabase& db) : iDb(db) {
        auto data = iDb.GetCacheData();
        std::lock_guard<std::recursive_mutex> lock(iMutex);

        for(const auto& v: data) {
            iCache[v.first] = std::make_pair(v.second, std::chrono::system_clock::now());
        }

        iHandle = std::async(
            std::launch::async,
            &Cache::refreshCashe, this);
    }

    ~Cache()
    {
        Dispose();
    }

    bool begin_transaction() 
    { 
        try
        {
            return iDb.begin_transaction();
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }    
        return false; 
    }

    bool commit_transaction() 
    { 
        try
        {
            return iDb.commit_transaction();
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }    
        return false;  
    }

    bool abort_transaction() 
    { 
        try
        {
            return iDb.abort_transaction();
        }
        catch (std::exception& e)
        {
            std::cout << "exception: " << e.what() << std::endl;
        }    
        return false;  
    }

    std::string get(const std::string& key)
    {
        std::lock_guard<std::recursive_mutex> lock(iMutex);

        if(exist(key))
        {
            auto val = iCache[key];
            if(isExpired(val.second))
            {
                std::string valdbdata = iDb.get(key); 
                put(key, valdbdata);
                return iCache[key].first;
            }
            return iCache[key].first;
        }

        return nullptr;
    }
    
    std::string set(const std::string& key, const std::string& data)
    {
        std::lock_guard<std::recursive_mutex> lock(iMutex);

        if(begin_transaction())
        {
            // save to db code
            iDb.set(key, data);
            put(key, data);

            if(!commit_transaction())
            {
                abort_transaction();
            }
        }
        
        return iCache[key].first;
    }

    std::string remove(const std::string& key)
    {
        std::lock_guard<std::recursive_mutex> lock(iMutex);

        if (exist(key)) {
            auto val = iCache[key];
            iCache.erase(key);
            iDb.remove(key);
            return val.first;
        }

        return nullptr;
    }
private:

    bool exist(const std::string& key)
    {
        return (iCache.count(key) > 0);
    }
    
    void Dispose()
    {
        iCycle = false;
        iHandle.wait();
    }

    bool isExpired(const std::chrono::system_clock::time_point& date_time) 
    {
        return date_time + std::chrono::milliseconds(iTtl) <= std::chrono::system_clock::now();
    }

    void put(const std::string& key, const std::string& data)
    {
        iCache[key] = std::make_pair(data,std::chrono::system_clock::now());
    }

    void refreshCashe() {
        while (iCycle) {
            std::this_thread::sleep_for(std::chrono::milliseconds(15000));
            std::lock_guard<std::recursive_mutex> lock(iMutex);

            std::unordered_map<std::string, std::pair<std::string, std::chrono::system_clock::time_point>> tmpCache = iCache;
            for (auto& item: tmpCache) {
                if (isExpired(item.second.second)) {
                    std::string valueData = iDb.get(item.first);
                    put(item.first, valueData);
                }
            }
        }
    }

    std::recursive_mutex iMutex;
    std::unordered_map<std::string, std::pair<std::string, std::chrono::system_clock::time_point>> iCache;
    std::future<void> iHandle;
    IDatabase& iDb;
    volatile bool iCycle = true;
    const int iTtl = 30000;
};

int main() {
    class Database : public IDatabase {
    public:
        virtual bool begin_transaction() override { return true; }
        virtual bool commit_transaction() override { return true; }
        virtual bool abort_transaction() override { return true; }
        virtual std::string get(const std::string& key) override { return "data for " + key; }
        virtual std::string set(const std::string& key, const std::string& data) override { return "OK"; }
        virtual std::string remove(const std::string& key) override { return "OK"; }
        virtual std::map<std::string, std::string> GetCacheData() override {
            return {{"key1", "data1"}, {"key2", "data2"}};
        }
    };

    Database db;
    Cache cache(db);

    // Test
    std::cout << "Getting key1: " << cache.get("key1") << std::endl;
    std::cout << "Setting key3: " << cache.set("key3", "data3") << std::endl;
    std::cout << "Removing key2: " << cache.remove("key2") << std::endl;

    return 0;
}
