#include <iostream>

#include <list>
#include <unordered_map>
#include <deque>
#include <vector>


template <typename key_t, typename item_t>
struct OPT_cache
{
private:
    std::size_t capacity;
    std::size_t amount_of_items;
    std::size_t hits_counter;

    using request_t = typename std::pair<key_t, item_t>;
    using index_t   = typename std::size_t;

    using IndexMapType = typename std::unordered_map<key_t, std::deque<index_t>>;
    IndexMapType map_future_item_index;

    using CacheMapType = typename std::unordered_map<key_t, item_t>;
    CacheMapType cache_map;

    using input_vector = typename std::vector<request_t>;
    input_vector items;

public:
    OPT_cache() : hits_counter(0), capacity(0) {}
    OPT_cache(std::size_t input_capacity, std::size_t input_amount_of_items, const input_vector &input_items) 
             : capacity(input_capacity), amount_of_items(input_amount_of_items), items(input_items) 
    {
        items.reserve(input_amount_of_items);  
        load_map_of_future();  
    }
    
    bool add_cache(const key_t &key, const item_t &item)
    {


    }

    void run_optimal_cache()
    {

    }
    
    item_t get_item(const key_t &key) const
    {

    }

    std::size_t get_hit_count() const
    {
        return hits_counter;
    }

    ~OPT_cache() 
    {
        map_future_item_index.clear();
    }

private:
    void load_map_of_future()
    {
        for (index_t i = 0; i < items.size(); i++)
        {
            key_t key = items[i].first;
            map_future_item_index[key].push(i);        
        }
    }
};