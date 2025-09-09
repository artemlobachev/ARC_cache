#ifndef ARC_CACHE_HPP
#define ARC_CACHE_HPP

#include <cstddef>
#include <iostream>
#include <list>
#include <cstddef>
#include <unordered_map>

template <typename key_t, typename item_t> struct ARC_cache
{
    ssize_t capacity;
    ssize_t hits_counter;

    private:
        const ssize_t POISON;

        ssize_t adapt_param;

        enum class list_t
        {
            FIRST_OCCUR_LIST,
            FREQ_OCCUR_LIST,
            FIRST_OCCUR_LIST_GHOST,
            FREQ_OCCUR_LIST_GHOST
        };
        
        std::list<item_t> list_first,
                          list_frequent,
                          list_first_ghost,
                          list_frequent_ghost;

        std::unordered_map<key_t, std::pair<list_t, typename std::list<item_t>::iterator>> cache_map;

        void adapt_first_ghost()  
        {
            double ratio_betwwen_ghosts_size = list_frequent_ghost.size() / list_first_ghost.size();
            double max_ = std::max(1.0, ratio_betwwen_ghosts_size);

            adapt_param = std::min(adapt_param + max_, static_cast<double>(capacity));
        }

        void adapt_second_ghost() 
        {
            double ratio_betwwen_ghosts_size = list_first_ghost.size() / list_first_ghost.size();
            double max_ = std::max(1.0, ratio_betwwen_ghosts_size);

            adapt_param = std::max(adapt_param - max_, 0.0);
        }

        inline bool replace_condition() const
        {
            const ssize_t list_size_tmp = list_first.size();

            bool is_list_not_empty = list_size_tmp >= 1;
            bool is_list_adaptive_enough = list_size_tmp >  adapt_param || 
                                          (list_size_tmp == adapt_param && !list_frequent_ghost.empty());

            return is_list_not_empty && is_list_adaptive_enough;
        }

        void adapt_and_replace()
        {
            const size_t list_size_tmp = list_first.size();

            if (replace_condition())
                list_first_ghost.insert(list_first.pop_back());
                
            else
                list_frequent_ghost.insert(list_frequent.pop_back());
        }

    public:
        ARC_cache(ssize_t capacity) : capacity(capacity), hits_counter(0), adapt_param(0) {}

        bool cache_update(key_t key);        
        ssize_t get_hits() const { return hits_counter;}

        ~ARC_cache()
        {
            list_first.clear();
            list_frequent.clear();
            list_first.clear();
            list_frequent.clear();

            cache_map.clear();        
        }
};

#endif