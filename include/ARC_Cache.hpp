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

    std::size_t capacity;
    std::size_t hits_counter;
    double adapt_param;
    
    ListType list_first,       list_frequent;
    ListType list_first_ghost, list_frequent_ghost; 

    using HashmapType = typename std::unordered_map<key_t, LocationInfo>;
    using HashmapIter = typename HashmapType::iterator;

    HashmapType hashmap;

    const std::size_t STD_CAPACITY = 64;

    void adapt_first_ghost()
    {
        assert(list_first_ghost.size());

        double ratio_between_ghosts_size = static_cast<double>(list_frequent_ghost.size()) / list_first_ghost.size();
        double max_ = std::max(1.0, ratio_between_ghosts_size);

        adapt_param = std::min(adapt_param + max_, static_cast<double>(capacity));
    }

    void adapt_second_ghost() 
    {
        assert(list_frequent_ghost.size());
        
        const double ratio_between_ghosts_size = static_cast<double>(list_first_ghost.size()) / list_frequent_ghost.size();
        const double max_ = std::max(1.0, ratio_between_ghosts_size);

        adapt_param = std::max(adapt_param - max_, 0.0);
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
        const std::size_t list_size_tmp = list_first.size(); // size1

        bool is_list_not_empty = list_size_tmp >= 1;
        bool is_list_adaptive_enough = list_size_tmp > static_cast<std::size_t>(adapt_param) || 
                                        (list_size_tmp == static_cast<std::size_t>(adapt_param) && !list_frequent_ghost.empty());

        return is_list_not_empty && is_list_adaptive_enough;
    }

    inline void replace_first_and_ghost()
    {
        assert(!list_first.empty());

        const CacheEntry tail_element = list_first.back();
        list_first.pop_back();

        list_first_ghost.push_front(tail_element);
        update_hashmap(tail_element.key, ListLocation::FIRST_LIST_GHOST, list_first_ghost.begin());
    }

    inline void replace_frequent_and_ghost()
    {   
        assert(!list_frequent.empty());
        
        const CacheEntry tail_element = list_frequent.back();
        list_frequent.pop_back();

        list_frequent_ghost.push_front(tail_element);
        update_hashmap(tail_element.key, ListLocation::FREQUENT_LIST_GHOST, list_frequent_ghost.begin());
    }

    void replace_for_adapt()
    {
        if (not_empty_and_adaptive()){ replace_first_and_ghost();}
        else                          replace_frequent_and_ghost();
    }

    inline void update_hashmap(const key_t &key, ListLocation location, ListIter list_it)
    {
        hashmap[key] = LocationInfo(location, list_it);
    }

    bool handle_existing_item(const HashmapIter &hashmap_it, const key_t &key)
    {  
        //  dump();
        ListLocation location  = hashmap_it->second.location;
        ListIter list_iterator = hashmap_it->second.list_iter;

        switch(location)
        {
            case ListLocation::FIRST_LIST:
                // hashmap_dump(hashmap);
                list_frequent.push_front(*list_iterator);
                list_first.erase(list_iterator);

                update_hashmap(key, ListLocation::FREQUENT_LIST, list_frequent.begin());                    
                hits_counter++;
                return true;

            case ListLocation::FREQUENT_LIST: 
                hits_counter++;
                return true;

            case ListLocation::FIRST_LIST_GHOST:
                adapt_first_ghost();
                replace_for_adapt();

                list_frequent.push_front(*list_iterator);
                list_first_ghost.erase(list_iterator);

                update_hashmap(key, ListLocation::FREQUENT_LIST, list_frequent.begin());
                return false;

            case ListLocation::FREQUENT_LIST_GHOST:
                adapt_second_ghost();
                replace_for_adapt();

                list_frequent.push_front(*list_iterator);
                list_frequent_ghost.erase(list_iterator);

                update_hashmap(key, ListLocation::FREQUENT_LIST, list_frequent.begin());
                return false;
            
            case ListLocation::NOT_FOUND:
                //on_debug
                return false;
        };

        return false;
    }

    inline void handle_cache_overflow()
    {
        std::size_t list1_size   = list_first.size();
        std::size_t list1gh_size = list_first_ghost.size();
        std::size_t list2_size   = list_frequent.size();
        std::size_t list2gh_size = list_frequent_ghost.size();

        std::size_t sum_size_lists = list1_size + list2_size + list1gh_size + list2gh_size;
        if (list1_size + list1gh_size == capacity)
        {
            if (list1_size < capacity)
            {
                if (!list_first_ghost.empty())
                {
                    hashmap.erase(list_first_ghost.back().key);
                    list_first_ghost.pop_back();
                }
                replace_for_adapt();
            }
            else
            {
                if (!hashmap.empty())
                {
                    hashmap.erase(list_first.back().key);
                    list_first.pop_back();
                }
            }
        }
        else if (list1_size + list1gh_size < capacity && sum_size_lists >= capacity)      
        {    
            if (sum_size_lists == 2 * capacity)
            { 
                if (!list_frequent_ghost.empty())
                {    
                    hashmap.erase(list_frequent_ghost.back().key);
                    list_frequent_ghost.pop_back();       
                }
            }
            replace_for_adapt();
        }
    }

    void add_new_item(const key_t &key, const item_t &item)
    {
        handle_cache_overflow();

        list_first.push_front(CacheEntry(key, item));
        update_hashmap(key, ListLocation::FIRST_LIST, list_first.begin());
    }


    void hashmap_dump(const HashmapType &hashmap) const
    {
        print_header("HASHMAP DUMP");

        for (const auto &pair : hashmap) // pair = hashmap.begin -> hashmap.end() <=> pair - hashmap_it
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

            std::cout << "[ item" << i++ <<" = " << item <<"(hashmap key: " << key <<  ") ]";
            if (i < size) std::cout << "<==>";
        }

        std::cout.put('\n');
        print_separator();
    }

public:
    ARCCache() : capacity(0), hits_counter(0), adapt_param(0.0)  {}

    ARCCache(std::size_t capacity) : capacity(capacity), hits_counter(0), adapt_param(0.0) {
        if (capacity == 0)
        {
            std::cout << "Capacity must be greater then 0\n capacity = STD_CAPACITY = " << STD_CAPACITY << std::endl;
        }

    }

    item_t get_item(const key_t &key) const
    {
        HashmapIter hashmap_it = hashmap.find(key);
        
            if (hashmap_it == hashmap.end()) return item_t();

        return hashmap_it->second.list_iter->item;
    }

    bool add_cache(const key_t &key, const item_t &item)
    {
        assert(capacity > 0);
        
        HashmapIter hashmap_it = hashmap.find(key); 

        if (hashmap_it != hashmap.end()) 
            return handle_existing_item(hashmap_it, key);
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
        list_first.clear();
        list_frequent.clear();
        list_first_ghost.clear();
        list_frequent_ghost.clear();

        hashmap.clear();
    }

    std::size_t get_hit_count() const { return hits_counter; }

    void dump() const
    {
        print_header("Adaptive replacement cache DUMP");
        
        std::cout << "capacity: " << capacity << std::endl;
        std::cout << "hit count: " << hits_counter << std::endl;
        std::cout << "adadptive parametr:" << static_cast<double>(adapt_param) << std::endl;

        hashmap_dump(hashmap);

        list_dump(list_first, ListLocation::FIRST_LIST);
        list_dump(list_frequent, ListLocation::FREQUENT_LIST);
        list_dump(list_first_ghost, ListLocation::FIRST_LIST_GHOST);
        list_dump(list_frequent_ghost, ListLocation::FREQUENT_LIST_GHOST);

        print_separator();
    }
};

#endif