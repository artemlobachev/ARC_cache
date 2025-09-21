#include <vector>

#include "../include/optimal/optimal_cache.hpp"
#include "../include/utils/driver/driver.hpp"


int main()
{
    HtmlLogger::init("optimal_cache_log");

    ssize_t capacity = 0;
    ssize_t amount_numbers = 0;
    
    std::cin >> capacity >> amount_numbers;

    CacheDriver<ssize_t, ssize_t> driver;
    auto optimal_cache_requests = driver.generate_requests(amount_numbers);

    OPT_cache<ssize_t, ssize_t> optimal_cache(capacity);
    driver.run_cache(optimal_cache, optimal_cache_requests);

    LOG_INFO("DOLBAEB", "HELLO\n", 5);
    HtmlLogger::close();
}