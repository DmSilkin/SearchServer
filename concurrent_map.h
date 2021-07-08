#pragma once
#include <map>
#include <mutex>
#include <vector>

using namespace std;

template <typename K, typename V>
class ConcurrentMap {
public:
    static_assert(is_integral_v<K>, "ConcurrentMap supports only integer keys");

    struct Access {
        lock_guard<mutex> guard;
        V& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count) : buckets_count_(bucket_count), mutex_vec_(bucket_count) {}

    Access operator[](const K& key) {
        int ind = static_cast<uint64_t>(key) % buckets_count_;
        {
            lock_guard guard(mutex_vec_[ind]);
            if (concurrent_map_.find(key) == concurrent_map_.end()) {
                lock_guard guard(temp_mutex_);
                concurrent_map_[key] = V();
            }
        }
        return { lock_guard(mutex_vec_[ind]), concurrent_map_[key] };
    }

    map<K, V> BuildOrdinaryMap() {
        return move(concurrent_map_);
    };
private:
    int buckets_count_;
    map<K, V> concurrent_map_;
    vector<mutex> mutex_vec_;
    mutex temp_mutex_;
};