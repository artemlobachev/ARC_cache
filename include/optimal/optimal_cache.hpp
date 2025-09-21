#ifndef OPTIMAL_CACHE_HPP 
#define OPTIMAL_CACHE_HPP

#include <unordered_map>
#include <deque>
#include <vector>
#include <iostream>
#include <cstdint>

#include "../CacheInterface.hpp"
#include "../utils/logger/logger.hpp"

template <typename key_t, typename item_t>
struct OPT_cache : CacheInterface<key_t, item_t>
{
private:
    using index_t   = ssize_t;
    using request_t = typename std::pair<key_t, item_t>;
    using IndexMapType = typename std::unordered_map<key_t, std::deque<index_t>>;
    using CacheMapType = typename std::unordered_map<key_t, item_t>;
    using input_vector = typename std::vector<request_t>;

    static constexpr key_t POISON_KEY = static_cast<ssize_t>(0XEDAA);
    static constexpr index_t INF_INDEX = UINT64_MAX;


    ssize_t capacity_;
    ssize_t hits_counter_;

    IndexMapType map_future_;
    CacheMapType cache_map_;


public:
    explicit OPT_cache() : capacity_(0), hits_counter_(0) {}
    explicit OPT_cache(ssize_t input_capacity) 
             : capacity_(input_capacity), hits_counter_(0)
    {
        LOG_INFO("OPT_Cache", "Cache initialized with capacity: ", input_capacity);
    }

    ~OPT_cache()
    {}

    ssize_t run_cache(const input_vector &key_items) override
    {
        if (capacity_ <= 0LL) 
        {
            LOG_ERROR("OPT cache", "capacity is INVALID. WE STOP IT");
            return 0;
        }
        load_map_of_future(key_items);

        //todo: impl add_cache() and add to cache interface
        for (ssize_t i = 0; i < key_items.size(); i++)
        {
            const auto &[curr_key, curr_item] = key_items[i];

            if (!map_future_[curr_key].empty())
                map_future_[curr_key].pop_front();

            if (cache_map_.find(curr_key) != cache_map_.end())
                hits_counter_++;

            else if (cache_map_.size() < capacity_)
                cache_map_[curr_key] = curr_item;

            else
            {
                bool should_be_replace = remove_farest(curr_key);
                if (should_be_replace) cache_map_[curr_key] = curr_item;
            }
        }

        return hits_counter_;
    }

    inline void dump() const override
    {
        print_hit_count();
    } 

    inline ssize_t get_hit_count() const override
    {
        return hits_counter_;
    }

    inline void print_hit_count() const override
    {
        std::cout << "hits: " << hits_counter_ << std::endl;
    }

private:
    bool remove_farest(const key_t &insert_key)
    {
        index_t farthest_next_index = 0;
        key_t   farthest_key  = POISON_KEY;
        const index_t next_insert_index = (map_future_[insert_key].empty()) ?  
                                           INF_INDEX : map_future_[insert_key].front();
        
        for (const auto &[key_in_cache, item] : cache_map_)
        {
            if (map_future_[key_in_cache].empty())
            {
                farthest_key = key_in_cache;
                break;
            }
            else 
            {
                index_t next_index = map_future_[key_in_cache].front();
                if (next_index > farthest_next_index)
                {
                        farthest_next_index = next_index;
                        farthest_key        = key_in_cache;
                }
            }
        }
        if (farthest_next_index > next_insert_index && farthest_key != POISON_KEY)
        {
            cache_map_.erase(farthest_key);
            return true;
        }
        else 
            return false;
    }

    inline void cache_map_dump() const
    {
        for (const auto &pair : cache_map_)
        {
            key_t key_in_cache = pair.first;
            item_t item_in_cache = pair.second;

            std::cout << "key: " << key_in_cache << " item: " << item_in_cache << std::endl;
        }
    }

    void load_map_of_future(const input_vector &key_items)
    {
        for (index_t i = 0; i < key_items.size(); i++)
            map_future_[key_items[i].first].push_back(i + 1);        
    }
};

#endif
