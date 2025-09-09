#include <cassert>

#include "../include/ARC_Cache.hpp"
#include <string.h>

template <typename key_t, typename item_t>
bool ARC_cache<key_t, item_t>::cache_update(key_t key)
{
    auto map_iterator = cache_map.find(key); 

    if (map_iterator != cache_map.end())
    {
        // TODO: CHANGE THIS CODE ON handle_in() or smth

        list_t list_type     = map_iterator->second.first;
        auto   list_iterator = map_iterator->second.second;

        switch(list_type)
        {
                case list_t::FIRST_OCCUR_LIST:
                    list_first.erase(list_iterator);
                    list_frequent.insert(list_iterator);
                    
                    hits_counter++;
                    return true;

                case list_t::FREQ_OCCUR_LIST: 
                    hits_counter++;
                    return true;

                case list_t::FIRST_OCCUR_LIST_GHOST:
                    adapt_first_ghost();
                    return false;

                case list_t::FIRST_OCCUR_LIST_GHOST:
                    adapt_second_ghost();
                    return false;

        };
    }
    ssize_t sum_size_lists = list_first.size() + list_frequent.size(); 

    assert(sum_size_lists > capacity);

    if (sum_size_lists < capacity)
        list_first.insert(key);

    if (sum_size_lists == capacity)
        adapt_and_replace();
}