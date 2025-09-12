#include <iostream>
#include <vector>

#include "../include/optimal_cache.hpp"

int main()
{
    std::size_t capacity = 0;
    std::size_t amount_numbers = 0;

    std::cin >> capacity >> amount_numbers; 

    std::vector<std::pair<int, int>> input_items(amount_numbers);

    for (int i = 0; i < amount_numbers;i++)
    {
        std::cin >> input_items[i].first;
        input_items[i].second = input_items[i].first;
    }

    OPT_cache<int, int> cache(capacity, amount_numbers, input_items);

    cache.run_optimal_cache();
}