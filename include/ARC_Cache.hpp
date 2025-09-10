#ifndef ARCCache_HPP
#define ARCCache_HPP

#include <cstddef>
#include <iostream>
#include <list>
#include <unordered_map>
#include <cassert>

template <typename key_t, typename item_t> struct ARCCache
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

        void adapt_first_ghost()
        {
            assert(list_first_ghost.size());
            
            print_header("heogeronegre");

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

            const CacheEntry &tail_element = list_first.back();
            list_first_ghost.push_front(tail_element);
            update_hashmap(tail_element.key, ListLocation::FIRST_LIST_GHOST, list_first_ghost.begin());
            list_first.pop_back();
        }

        inline void replace_frequent_and_ghost()
        {   
            assert(!list_frequent.empty());
            
            auto tail_element = list_frequent.back();
            list_frequent.pop_back();

            list_frequent_ghost.push_front(tail_element);
            update_hashmap(tail_element.key, ListLocation::FREQUENT_LIST_GHOST, list_frequent_ghost.begin());
        }

        void adapt_and_replace()
        {
            if (not_empty_and_adaptive()){ replace_first_and_ghost();}
            else                          replace_frequent_and_ghost();
        }

   /*     inline void update_hashmap(ListLocation location , const key_t &key)
        {
            switch(location)
            {
                case ListLocation::FIRST_LIST:
                    hashmap[key] = LocationInfo(location, list_first.begin());
                    break;

                case ListLocation::FREQUENT_LIST:
                    hashmap[key] = LocationInfo(location, list_frequent.begin());
                    break;

                case ListLocation::FIRST_LIST_GHOST:
                    hashmap[key] = LocationInfo(location, list_first_ghost.begin())
                
            }

            LocationInfo loc_info(ListLocation::FIRST_LIST, list_first.begin());
            hashmap[key] = LocationInfo(location, );
        } */

        void inline update_hashmap(const key_t &key, ListLocation location, ListIter list_it)
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
                    list_first.pop_back();
                    list_frequent.push_front(*list_iterator);

                    update_hashmap(key, ListLocation::FREQUENT_LIST, list_frequent.begin());                    
                    hits_counter++;
                    return true;

                case ListLocation::FREQUENT_LIST: 
                    hits_counter++;
                    return true;

                case ListLocation::FIRST_LIST_GHOST:
                    adapt_first_ghost();
                    return false;

                case ListLocation::FREQUENT_LIST_GHOST:
                    adapt_second_ghost();
                    return false;
                
                case ListLocation::NOT_FOUND:
                    //on_debug
                    return false;
            };

            return false;
        }

        void add_new_item(const key_t &key, const item_t &item)
        {
            std::size_t sum_size_lists = list_first.size() + list_frequent.size();

            if (sum_size_lists > capacity)  { dump(); return;}
            
            if (sum_size_lists == capacity) 
            { 
                if (key == 3) dump();
                adapt_and_replace(); 
                list_first.push_front(CacheEntry(key, item)); 
                update_hashmap(key, ListLocation::FIRST_LIST, list_first.begin());
            }
            else
            {
                list_first.push_front(CacheEntry(key, item));
                update_hashmap(key, ListLocation::FIRST_LIST, list_first.begin());
        
            }
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

        inline void list_header(ListLocation which_list) const
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
            }
        }

        void list_dump(ListType list, ListLocation which_list) const
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

        ARCCache(std::size_t capacity) : capacity(capacity), hits_counter(0), adapt_param(0.0) {}

        bool add(const key_t &key, const item_t &item)
        {
            return false;
        }

        bool add_key_is_item(const key_t &key)
        {
            HashmapIter hashmap_it = hashmap.find(key); 

         //   dump();
            if (hashmap_it != hashmap.end()) return handle_existing_item(hashmap_it, key);
            else                           { add_new_item(key, key); return false; }
        }       
        ~ARCCache()
        {
            list_first.clear();
            list_frequent.clear();
            list_first_ghost.clear();
            list_frequent_ghost.clear();

            hashmap.clear();

            //ON_DEBUG(DUMP());
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