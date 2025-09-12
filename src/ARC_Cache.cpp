#include <vector>

#include "../include/ARC_Cache.hpp"

int main()
{
    std::size_t capacity = 0;
    std::size_t amount_numbers = 0;
    
    std::cin >> capacity >> amount_numbers;

    ARCCache<std::size_t, std::size_t> cache(capacity);

    std::vector<std::pair<std::size_t, std::size_t>> input_key_item(amount_numbers); 

    for (std::size_t i = 0; i < amount_numbers; i++)
    {
        size_t key = 0;
        std::cin >> key;
        input_key_item[i] = {key, key};
    }

    std::size_t hits_counter = cache.run_ARC_cache(input_key_item);
    std::cout << "hits: " << hits_counter << std::endl;  
  //  cache.dump();
}