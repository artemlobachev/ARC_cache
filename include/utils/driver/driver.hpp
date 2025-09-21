#ifndef DRIVER_HPP
#define DRIVER_HPP

#include <iostream>
#include <vector>

#include "../logger/logger.hpp"
#include "../../CacheInterface.hpp"

template <typename key_t, typename item_t>
class CacheDriver
{
private:
    static constexpr ssize_t STD_VECTOR_CAPACITY = 16;

    using request_t = typename std::pair<key_t, item_t>;
    std::vector<request_t> requests;

public:
    CacheDriver() { }
    ~CacheDriver() { }

    const std::vector<request_t> &generate_requests(ssize_t amount_numbers)
    {
        handle_vector_size(requests, amount_numbers);
         for (ssize_t i = 0; i < requests.size(); i++)
         {
            ssize_t key = 0;
            std::cin >> key;
            requests[i] = {key, key};
         }

        return requests;
    }

    void run_cache(CacheInterface<key_t, item_t> &cache, std::vector<request_t> requests)
    {
        cache.run_cache(requests);
        cache.print_hit_count();
    }

    void compare_caches(CacheInterface<key_t, item_t> &cache1, 
                        CacheInterface<key_t, item_t> &cache2,
                        std::vector<request_t> requests       )
    {
        std::cout << "=== Comparing Caches ===\n";
        
        run_cache(cache1, requests);
        
        std::cout << std::endl;
        
        run_cache(cache2, requests);
    }

private:
    bool handle_vector_size(std::vector<request_t> &vec, const ssize_t &size)
    {
        if (size > vec.max_size())
        {
            LOG_WARNING("Cache driver", "INPUT SIZE IS ", size, 
                                              "\n MORE THEN MAX SIZE: ", vec.max_size(),
                                              "HANDLE IT AND SET ",      STD_VECTOR_CAPACITY);

            vec.resize(STD_VECTOR_CAPACITY);

            return false;
        }

        if (size <= 0)
        {   
            LOG_WARNING("Cache driver", "INVALID INPUT SIZE(or zero): ",    size,
                                        "HANDLE IT AND SET STD CAPACITY: ", STD_VECTOR_CAPACITY);

            vec.resize(STD_VECTOR_CAPACITY);

            return false;
        }

        vec.resize(size);
        return true;
    }

};

#endif 