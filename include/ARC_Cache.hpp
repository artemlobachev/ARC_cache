#ifndef ARCCache_HPP
#define ARCCache_HPP

#include <cstddef>
#include <iostream>
#include <list>
#include <vector>
#include <unordered_map>
#include <cassert>

template <typename key_t, typename item_t> 
struct ARCCache
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

    std::size_t capacity_;
    std::size_t hits_counter_;
    double adapt_param_;
    
    ListType list_first_,       list_frequent_;
    ListType list_first_ghost_, list_frequent_ghost_; 

    using CacheMapType = typename std::unordered_map<key_t, LocationInfo>;
    using CacheMapIter = typename CacheMapType::iterator;

    CacheMapType cache_map_;

    const std::size_t STD_CAPACITY = 64;

    void adapt_first_ghost()
    {
        assert(list_first_ghost_.size());

        double ratio_between_ghosts_size = static_cast<double>(list_frequent_ghost_.size()) / 
                                           static_cast<double>(list_first_ghost_.size());
        double max_ = std::max(1.0, ratio_between_ghosts_size);

        adapt_param_ = std::min(adapt_param_ + max_, static_cast<double>(capacity_));
    }

    void adapt_second_ghost() 
    {
        assert(list_frequent_ghost_.size());
        
        const double ratio_between_ghosts_size = static_cast<double>(list_first_ghost_.size()) / 
                                                 static_cast<double>(list_frequent_ghost_.size());
        const double max_ = std::max(1.0, ratio_between_ghosts_size);

        adapt_param_ = std::max(adapt_param_ - max_, 0.0);
    }

    inline void print_header(const std::string &word) const
    {
        std::cout << "============ " << word << " ============\n\n";
    }

    inline void print_separator() const
    {
        std::cout << "\n=======================================\n\n";
    }

    inline void print_location(ListLocation loc) const
    {
        switch(loc)
        {
            case ListLocation::FIRST_LIST:
                std::cout << "First occur list";
                break;

            case ListLocation::FREQUENT_LIST:
                std::cout << "Frequent occur list";
                break;

            case ListLocation::FIRST_LIST_GHOST:
                std::cout << "First occur ghost_list";
                break;

            case ListLocation::FREQUENT_LIST_GHOST:
                std::cout << "Frequent occur ghost_list";
                break;

            case ListLocation::NOT_FOUND:
                std::cout << "ELEMENT WASN`T FOUND IN ANY LIST";
                break;
        }
    }

    inline bool not_empty_and_adaptive() const
    {
        const std::size_t list_size_tmp = list_first_.size(); // size1

        bool is_list_not_empty = list_size_tmp >= 1;
        bool is_list_adaptive_enough = list_size_tmp > static_cast<std::size_t>(adapt_param_) || 
                                        (list_size_tmp == static_cast<std::size_t>(adapt_param_) && !list_frequent_ghost_.empty());

        return is_list_not_empty && is_list_adaptive_enough;
    }

    inline void replace_first_and_ghost()
    {
        assert(!list_first_.empty());

        const CacheEntry tail_element = list_first_.back();
        list_first_.pop_back();

        list_first_ghost_.push_front(tail_element);
        update_cache_map(tail_element.key, ListLocation::FIRST_LIST_GHOST, list_first_ghost_.begin());
    }

    inline void replace_frequent_and_ghost()
    {   
        assert(!list_frequent_.empty());
        
        const CacheEntry tail_element = list_frequent_.back();
        list_frequent_.pop_back();

        list_frequent_ghost_.push_front(tail_element);
        update_cache_map(tail_element.key, ListLocation::FREQUENT_LIST_GHOST, list_frequent_ghost_.begin());
    }

    void replace_for_adapt()
    {
        if (not_empty_and_adaptive()){ replace_first_and_ghost();}
        else                          replace_frequent_and_ghost();
    }

    inline void update_cache_map(const key_t &key, ListLocation location, ListIter list_it)
    {
        cache_map_[key] = LocationInfo(location, list_it);
    }

    bool handle_existing_item(const CacheMapIter &cache_map_it, const key_t &key)
    {  
        //  dump();
        ListLocation location  = cache_map_it->second.location;
        ListIter list_iterator = cache_map_it->second.list_iter;

        switch(location)
        {
            case ListLocation::FIRST_LIST:
                // cache_map_dump(cache_map);
                list_frequent_.push_front(*list_iterator);
                list_first_.erase(list_iterator);

                update_cache_map(key, ListLocation::FREQUENT_LIST, list_frequent_.begin());                    
                hits_counter_++;
                return true;

            case ListLocation::FREQUENT_LIST: 
                hits_counter_++;
                return true;

            case ListLocation::FIRST_LIST_GHOST:
                adapt_first_ghost();
                replace_for_adapt();

                list_frequent_.push_front(*list_iterator);
                list_first_ghost_.erase(list_iterator);

                update_cache_map(key, ListLocation::FREQUENT_LIST, list_frequent_.begin());
                return false;

            case ListLocation::FREQUENT_LIST_GHOST:
                adapt_second_ghost();
                replace_for_adapt();

                list_frequent_.push_front(*list_iterator);
                list_frequent_ghost_.erase(list_iterator);

                update_cache_map(key, ListLocation::FREQUENT_LIST, list_frequent_.begin());
                return false;
            
            case ListLocation::NOT_FOUND:
                //on_debug
                return false;
        };

        return false;
    }

    inline void handle_cache_overflow()
    {
        std::size_t list1_size   = list_first_.size();
        std::size_t list1gh_size = list_first_ghost_.size();
        std::size_t list2_size   = list_frequent_.size();
        std::size_t list2gh_size = list_frequent_ghost_.size();

        std::size_t sum_size_lists = list1_size + list2_size + list1gh_size + list2gh_size;
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
        print_header("cache_map DUMP");

        for (const auto &pair : cache_map) // pair = cache_map.begin -> cache_map.end() <=> pair - cache_map_it
        {
            const LocationInfo &loc_info = pair.second; // pair.second - iterator 
            const ListIter     &list_it  = loc_info.list_iter;
            
            std::cout << "key = " << pair.first << ": " << "item location: ";

            print_location(loc_info.location);
            
            if (list_it != ListIter())
            {
                std::cout << " item: " << list_it->item;
                std::cout << " key by cache info: " << list_it->key; 
            }
            else 
                std::cout << " INVALID ITERATOR";

            std::cout << std::endl;
        }
        print_separator();
    }

    inline void list_header(const ListLocation which_list) const
    {
        switch(which_list)
        {
            case ListLocation::FIRST_LIST:
                print_header("First occur list DUMP");
                break;

            case ListLocation::FREQUENT_LIST:
                print_header("Frequent occur list DUMP");
                break;

            case ListLocation::FIRST_LIST_GHOST:
                print_header("First occur list_ghost DUMP");
                break;

            case ListLocation::FREQUENT_LIST_GHOST:
                print_header("Frequent occur list_ghost DUMP");
                break;

            case ListLocation::NOT_FOUND:
                print_header("UNDEFINED LIST");
                break;
        }
    }

    void list_dump(const ListType &list, const ListLocation which_list) const
    {
        list_header(which_list);


        std::size_t i = 0;
        std::size_t size = list.size();
        for (const auto &cache : list) // pair => list.begin() == ListIter
        {
            item_t item = cache.item;
            key_t  key  = cache.key;

            std::cout << "[ item" << i++ <<" = " << item <<"(cache_map key: " << key <<  ") ]";
            if (i < size) std::cout << "<==>";
        }

        std::cout.put('\n');
        print_separator();
    }

public:
    ARCCache() : capacity_(0), hits_counter_(0), adapt_param_(0.0)  {}

    ARCCache(std::size_t capacity) : capacity_(capacity), hits_counter_(0), adapt_param_(0.0) {
        if (capacity == 0)
        {
            std::cout << "Capacity must be greater then 0\n capacity = STD_CAPACITY = " << STD_CAPACITY << std::endl;
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
    
    std::size_t run_ARC_cache(std::vector<std::pair<key_t, item_t>> input_key_item)
    {
        for (std::size_t i = 0; i < input_key_item.size(); i++)
            add_cache(input_key_item[i].first, input_key_item[i].second);

        return get_hit_count();
    }

    ~ARCCache()
    {
        list_first_.clear();
        list_frequent_.clear();
        list_first_ghost_.clear();
        list_frequent_ghost_.clear();

        cache_map_.clear();
    }

    std::size_t get_hit_count() const { return hits_counter_; }

    void dump() const
    {
        print_header("Adaptive replacement cache DUMP");
        
        std::cout << "capacity: " << capacity_ << std::endl;
        std::cout << "hit count: " << hits_counter_ << std::endl;
        std::cout << "adadptive parametr:" << static_cast<double>(adapt_param_) << std::endl;

        cache_map_dump(cache_map_);

        list_dump(list_first_, ListLocation::FIRST_LIST);
        list_dump(list_frequent_, ListLocation::FREQUENT_LIST);
        list_dump(list_first_ghost_, ListLocation::FIRST_LIST_GHOST);
        list_dump(list_frequent_ghost_, ListLocation::FREQUENT_LIST_GHOST);

        print_separator();
    }
};

#endif