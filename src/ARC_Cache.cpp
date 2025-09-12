#include "../include/ARC_Cache.hpp"

int main()
{
    std::size_t capacity = 0;
    std::size_t amount_numbers = 0;
    
    std::cin >> capacity >> amount_numbers;

    ARCCache<std::size_t, std::size_t> cache(capacity);

    for (std::size_t i = 0; i < amount_numbers; i++)
    {
        size_t key = 0;
        std::cin >> key;
        cache.add_cache(key, key);
        std::cout << cache.get_item(key) << std::endl;
    }

    cache.dump();
}