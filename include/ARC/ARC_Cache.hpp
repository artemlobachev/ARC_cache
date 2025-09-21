#ifndef ARCCache_HPP
#define ARCCache_HPP

#include <algorithm>
#include <iostream>
#include <list>
#include <vector>
#include <unordered_map>
#include <cassert>

#include "utils/logger.hpp"
#include "CacheInterface.hpp"

template <typename key_t, typename item_t> 
class ARCCache : public CacheInterface<key_t, item_t>
{
private:
    enum class ListLocation
    {
        FIRST_LIST,
        FREQUENT_LIST,
        FIRST_LIST_GHOST,
        FREQUENT_LIST_GHOST,
        NOT_FOUND
    };

    struct CacheEntry
    {
        key_t key;
        item_t item;

        CacheEntry() : key(), item() {}
        CacheEntry(const key_t &k, const item_t &i) : key(k), item(i) {}
    };

    using ListType = typename std::list<CacheEntry>;
    using ListIter = typename ListType::iterator;
    

    struct LocationInfo
    {
        ListLocation location;
        ListIter list_iter;
    
        LocationInfo() : location(ListLocation::NOT_FOUND), list_iter() {}
        LocationInfo(ListLocation loc, ListIter it) : location(loc), list_iter(it) {}    
    };

    ssize_t capacity_;
    ssize_t hits_counter_;
    double adapt_param_;
    
    ListType list_first_,       list_frequent_;
    ListType list_first_ghost_, list_frequent_ghost_; 

    using CacheMapType = typename std::unordered_map<key_t, LocationInfo>;
    using CacheMapIter = typename CacheMapType::iterator;

    CacheMapType cache_map_;

    static constexpr ssize_t STD_CAPACITY = 64;

    void adapt_ghost(ListLocation location)
    {
        assert(location != ListLocation::NOT_FOUND);

        const auto &nominator   = (location == ListLocation::FIRST_LIST_GHOST) 
                                ?  list_frequent_ghost_.size() : list_first_ghost_.size();

        const auto &denominator = (location == ListLocation::FIRST_LIST_GHOST) 
                                ? list_first_ghost_.size() : list_frequent_ghost_.size();
                                
        double ratio_between_ghosts_size = static_cast<double>(nominator) / 
                                           static_cast<double>(denominator);

        double max_  = std::max(1.0, ratio_between_ghosts_size);
        adapt_param_ += (location == ListLocation::FIRST_LIST_GHOST) ? max_ : -max_;
        adapt_param_ = std::clamp(adapt_param_, 0.0, static_cast<double>(capacity_)); 
    }

    inline char const *get_location(ListLocation loc) const
    {
        switch(loc)
        {
            case ListLocation::FIRST_LIST:
                return "First occur list";

            case ListLocation::FREQUENT_LIST:
                return "Frequent occur list";

            case ListLocation::FIRST_LIST_GHOST:
                return "First occur ghost_list";

            case ListLocation::FREQUENT_LIST_GHOST:
                return "Frequent occur ghost_list";

            case ListLocation::NOT_FOUND:
                return "UNDEFINED";
        }

        return "UNDEFINED";
    }

    inline bool not_empty_and_adaptive() const
    {
        const ssize_t list_size_tmp = list_first_.size(); // size1

        bool is_list_not_empty = list_size_tmp >= 1;
        bool is_list_adaptive_enough = list_size_tmp > static_cast<ssize_t>(adapt_param_) || 
                                        (list_size_tmp == static_cast<ssize_t>(adapt_param_) && !list_frequent_ghost_.empty());

        return is_list_not_empty && is_list_adaptive_enough;
    }

    inline void move_tail_to_front(
        ListType &src,
        ListType &dest,
        ListLocation dest_location)
    {
        assert(!src.empty());

        const CacheEntry tail_element = std::move(src.back());
        src.pop_back();

        dest.push_front(tail_element);
        update_cache_map(tail_element.key, dest_location, dest.begin());
    }

    void replace_for_adapt()
    {
        if (not_empty_and_adaptive())
            move_tail_to_front(list_first_, list_first_ghost_, ListLocation::FREQUENT_LIST);
        else
            move_tail_to_front(list_frequent_, list_frequent_, ListLocation::FREQUENT_LIST);
    }

    inline void update_cache_map(const key_t &key, ListLocation location, ListIter list_it)
    {
        cache_map_[key] = LocationInfo(location, list_it);
    }

    void move_to_dest_front(ListType &src, ListType &dest, 
        ListIter list_iterator, const ListLocation &location, const key_t &key)
    {
        CacheEntry element = std::move(*list_iterator);
        src.erase(list_iterator);

        dest.push_front(std::move(element));
        update_cache_map(key, location, dest.begin());
    }

    void move_to_frequent(ListLocation loc, const key_t &key, const ListIter &list_iterator)
    {
        const auto &tmp = *list_iterator;
        switch(loc)
        {
            case ListLocation::FIRST_LIST:
                move_to_dest_front(list_first_, list_frequent_, list_iterator, loc, key);
                break;

            case ListLocation::FIRST_LIST_GHOST:
                move_to_dest_front(list_first_ghost_, list_frequent_, list_iterator, loc, key);
                break;
            
            case ListLocation::FREQUENT_LIST_GHOST:
                move_to_dest_front(list_frequent_ghost_, list_frequent_, list_iterator, loc, key);
                break;
            
            case ListLocation::FREQUENT_LIST:
                return;

        }
        list_frequent_.push_front(tmp);
        update_cache_map(key, ListLocation::FREQUENT_LIST, list_frequent_.begin());
    }

    void handle_ghost(ListLocation location, key_t key, ListIter list_iterator)
    {
        adapt_ghost(location);
        replace_for_adapt();
        move_to_frequent(location, key, list_iterator);
    }

    bool handle_existing_item(const CacheMapIter &cache_map_it, const key_t &key)
    {      
        ListLocation location  = cache_map_it->second.location;
        ListIter list_iterator = cache_map_it->second.list_iter;

        switch(location)
        {
            case ListLocation::FIRST_LIST:
                move_to_frequent(location, key, list_iterator);             
                [[fallthrough]];
            case ListLocation::FREQUENT_LIST: 
                hits_counter_++;
                return true;

            case ListLocation::FIRST_LIST_GHOST:
                [[fallthrough]];
            case ListLocation::FREQUENT_LIST_GHOST:
                handle_ghost(location, key, list_iterator);
                return false;
                
            case ListLocation::NOT_FOUND:
                return false;
        };

        return false;
    }

    inline void handle_cache_overflow()
    {
        ssize_t list1_size   = list_first_.size();
        ssize_t list1gh_size = list_first_ghost_.size();
        ssize_t list2_size   = list_frequent_.size();
        ssize_t list2gh_size = list_frequent_ghost_.size();

        ssize_t sum_size_lists = list1_size + list2_size + list1gh_size + list2gh_size;
        if (list1_size + list1gh_size == capacity_)
        {
            if (list1_size < capacity_)
            {
                if (!list_first_ghost_.empty())
                {
                    cache_map_.erase(list_first_ghost_.back().key);
                    list_first_ghost_.pop_back();
                }
                replace_for_adapt();
            }
            else
            {
                if (!cache_map_.empty())
                {
                    cache_map_.erase(list_first_.back().key);
                    list_first_.pop_back();
                }
            }
        }
        else if (list1_size + list1gh_size < capacity_ && sum_size_lists >= capacity_)      
        {    
            if (sum_size_lists == 2 * capacity_)
            { 
                if (!list_frequent_ghost_.empty())
                {    
                    cache_map_.erase(list_frequent_ghost_.back().key);
                    list_frequent_ghost_.pop_back();       
                }
            }
            replace_for_adapt();
        }
    }

    void add_new_item(const key_t &key, const item_t &item)
    {
        handle_cache_overflow();

        list_first_.push_front(CacheEntry(key, item));
        update_cache_map(key, ListLocation::FIRST_LIST, list_first_.begin());
    }


    void cache_map_dump(const CacheMapType &cache_map) const
    {
   
        for (const auto &pair : cache_map)
        {
            const LocationInfo &loc_info = pair.second;
            const ListIter     &list_it  = loc_info.list_iter;

            LOG_DUMP("Cache map", 
                "key = ", pair.first, "\nitem location: ", get_location(loc_info.location));
            
            if (list_it != ListIter())
                LOG_DUMP("List iterator in cache map", " item: ", list_it->item,
                "\nkey by cache info: ", list_it->key); 
            else 
                LOG_DUMP("List iterator in cache map", " INVALID ITERATOR");
        }
    }

    inline char const * list_header(const ListLocation which_list) const
    {
        switch(which_list)
        {
            case ListLocation::FIRST_LIST:
                return "First occur list DUMP";

            case ListLocation::FREQUENT_LIST:
                return "Frequent occur list DUMP";

            case ListLocation::FIRST_LIST_GHOST:
                return "First occur list_ghost DUMP";

            case ListLocation::FREQUENT_LIST_GHOST:
                return "Frequent occur list_ghost DUMP";

            case ListLocation::NOT_FOUND:
                return "UNDEFINED LIST";
        }

        return "UNDEFINED LIST";
    }

    void list_dump(const ListType &list, const ListLocation which_list) const
    {
        ssize_t i = 0;
        ssize_t size = list.size();
        for (const auto &cache : list) 
        {
            item_t item = cache.item;
            key_t  key  = cache.key;

            LOG_DUMP(list_header(which_list), "[ item", i++, " = ", item, "(cache_map key: ", key,  ") ]");
        }
    }

public:
    explicit ARCCache() : capacity_(0), hits_counter_(0), adapt_param_(0.0)  
    {
 //       HtmlLogger::init("CacheDriver");
    }

    explicit ARCCache(ssize_t capacity) : capacity_(capacity), hits_counter_(0), adapt_param_(0.0) 
    {
        LOG_INFO("OPT_Cache", "Cache initialized with capacity: ", capacity);
        
        if (capacity <= 0)
        {
            LOG_WARNING("BAD INPUT", "Capacity is INVALID, set\n capacity = STD_CAPACITY = ", STD_CAPACITY);
            capacity_ = STD_CAPACITY;
        }

    }

    item_t get_item(const key_t &key) const
    {
        CacheMapIter cache_map_it = cache_map_.find(key);
        
            if (cache_map_it == cache_map_.end()) return item_t();

        return cache_map_it->second.list_iter->item;
    }

    bool add_cache(const key_t &key, const item_t &item)
    {
        if (capacity_ <= 0) return false;
        
        CacheMapIter cache_map_it = cache_map_.find(key); 

        if (cache_map_it != cache_map_.end()) 
            return handle_existing_item(cache_map_it, key);
        else                           
        { 
            add_new_item(key, item); 
            return false; 
        }
    }
    
    ssize_t run_cache(const std::vector<std::pair<key_t, item_t>> &input_key_item) override
    {
        if (capacity_ <= 0)
        {
            LOG_ERROR("ARC cache", "capacity is INVALID. STOP IT");
            return 0;
        }
        
        for (ssize_t i = 0; i < input_key_item.size(); i++)
            add_cache(input_key_item[i].first, input_key_item[i].second);

        return get_hit_count();
    }

    ~ARCCache() = default;
    //{ HtmlLogger::close(); }

    inline ssize_t get_hit_count() const override { return hits_counter_; }
    
    inline void print_hit_count() const override
    {
        std::cout << "hits: " << hits_counter_ << std::endl;
    }

    void dump() const override
    {

        LOG_DUMP("Adaptive replacement cache DUMP",
        "capacity: ", capacity_,
        "\nhit count: ", hits_counter_,
        "\nadadptive parametr:", static_cast<double>(adapt_param_));

        cache_map_dump(cache_map_);

        list_dump(list_first_, ListLocation::FIRST_LIST);
        list_dump(list_frequent_, ListLocation::FREQUENT_LIST);
        list_dump(list_first_ghost_, ListLocation::FIRST_LIST_GHOST);
        list_dump(list_frequent_ghost_, ListLocation::FREQUENT_LIST_GHOST);
    }
};

#endif