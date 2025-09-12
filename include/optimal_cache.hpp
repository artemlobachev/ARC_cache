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
    IndexMapType map_future;

    using CacheMapType = typename std::unordered_map<key_t, item_t>;
    CacheMapType cache_map;

    using input_vector = typename std::vector<request_t>;
    input_vector items = 0;

public:
    OPT_cache() : capacity(0), amount_of_items(0), hits_counter(0), items(0) {}
    OPT_cache(std::size_t input_capacity, std::size_t input_amount_of_items, const input_vector &input_items) 
             : capacity(input_capacity), amount_of_items(input_amount_of_items), hits_counter(0), items(input_items)
    {
        items.reserve(input_amount_of_items);  
        load_map_of_future();  
    }
    
/*    bool add_cache(const key_t &key, const item_t &item)
    {


    } */

    void run_optimal_cache()
    {
        for (std::size_t i = 0; i < items.size(); i++)
        {
            key_t  curr_key  = items[i].first;
            item_t curr_item = items[i].second;

            if (cache_map.find(curr_key) != cache_map.end())
            { 
                hits_counter++;
            }

            else if (cache_map.size() < capacity)
                    cache_map[curr_key] = curr_item;
            else    
            {
                remove_farest();
                cache_map[curr_key] = curr_item;
            }
        }
    }

    std::size_t get_hit_count() const
    {
        return hits_counter;
    }

    ~OPT_cache() 
    {
        map_future.clear();
    }

private:
    void remove_farest()
    {
        index_t farthest_next_id = -1;
        key_t   farthest_key  = -1;

        for (const auto &pair : cache_map)
        {
            key_t key_in_cache = pair.first;
            index_t closest_index = map_future[key_in_cache].front();
            
            if (map_future[key_in_cache].empty())
            {
                farthest_key = key_in_cache;
                break;
            }
            else if (closest_index > farthest_next_id)
            {
                farthest_next_id = closest_index;
                farthest_key  = key_in_cache;
            }
        }

        cache_map.erase(farthest_key);
      //  map_future[farthest_key].pop_front();
    }

    void load_map_of_future()
    {
        for (index_t i = 0; i < items.size(); i++)
        {
            key_t key = items[i].first;
            map_future[key].push_back(i);        
        }
    }
};