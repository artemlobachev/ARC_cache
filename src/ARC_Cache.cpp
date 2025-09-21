#include <vector>

#include "../include/ARC/ARC_Cache.hpp"
#include "../include/utils/driver/driver.hpp"


int main()
{
    HtmlLogger::init("arc_cache_log");

    LOG_INFO("DOLBAEB", "HELLO\n", 5);

    ssize_t capacity = 0;
    ssize_t amount_numbers = 0;
    
    std::cin >> capacity >> amount_numbers;

    CacheDriver<ssize_t, ssize_t> driver;
    auto arc_cache_requests = driver.generate_requests(amount_numbers);

    ARCCache<ssize_t, ssize_t> arc_cache(capacity);
    driver.run_cache(arc_cache, arc_cache_requests);

    LOG_INFO("DOLBAEB", "HELLO\n", 5);
    HtmlLogger::close();
}