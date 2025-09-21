#ifndef CACHE_INTERFACE_HPP
#define CACHE_INTERFACE_HPP

#include <iostream>
#include <vector>

template <typename key_t, typename item_t>
class CacheInterface 
{
public:
    virtual ssize_t run_cache(const std::vector<std::pair<key_t, item_t>>& requests) = 0;
    
    
    virtual ssize_t get_hit_count() const = 0;
    virtual void print_hit_count() const = 0;
    virtual void dump() const = 0;
};

#endif
