
#pragma once

#include <chrono>
#include <mutex>
#include <unordered_map>
#include <functional>
#include <thread>
#include <type_traits>

namespace cache
{
    // Проверка на наличие Hash функции у типа ключа
    template <typename T, typename = std::void_t<>>
    struct is_std_hashable : std::false_type
    {};

    template <typename T>
    struct is_std_hashable<T, std::void_t<decltype(std::declval<std::hash<T>>()(std::declval<T>()))>> : std::true_type
    {};


    template <class K, class T, class = typename std::enable_if<is_std_hashable<K>::value>::type>
    class TemporalCache
    {
        struct CacheItem
        {
            std::chrono::time_point<std::chrono::system_clock> lastTimeWrite;
            T value;
        };

        std::unordered_map<K, CacheItem> dataMap;
        std::chrono::seconds maxLifeTime;
        std::mutex mutex;

    public:
        TemporalCache(std::chrono::seconds _maxLifeTime, uint32_t _clearTimerIntevalS = 10) : maxLifeTime(_maxLifeTime)
        {
            clearTimerStart(std::bind(&TemporalCache<K, T>::clearOldCache, this), _clearTimerIntevalS * 1000);
        }

        TemporalCache(const TemporalCache &) = delete;
        const TemporalCache &operator=(const TemporalCache &) = delete;

        bool getValue(const K &key, T &value)
        {
            std::lock_guard<std::mutex> lock(mutex);

            auto it = dataMap.find(key);
            if (it != dataMap.end())
            {
                it->second.lastTimeWrite = std::chrono::system_clock::now();
                value = it->second.value;
                return true;
            }

            return false;
        }

        void setValue(const K &key, T value)
        {
            std::lock_guard<std::mutex> lock(mutex);
            dataMap[key] = { std::chrono::system_clock::now(), value };
        }

    private:
        void clearTimerStart(std::function<void(void)> func, uint32_t intervalMs)
        {
            std::thread([func, intervalMs]() { 
                while (true)
                { 
                    auto next = std::chrono::steady_clock::now() + std::chrono::milliseconds(intervalMs);
                    func();
                    std::this_thread::sleep_until(next);
                } })
                .detach();
        }

        void clearOldCache()
        {
            std::lock_guard<std::mutex> lock(mutex);
            for (auto it = dataMap.begin(); it != dataMap.end();)
            {
                const auto now = std::chrono::system_clock::now();
                if ((now - it->second.lastTimeWrite) > maxLifeTime)
                    it = dataMap.erase(it);
                else
                    ++it;
            }
        }
    };

} // namespace cache
