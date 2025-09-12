cmake_minimum_required(VERSION 3.10)
project(ARC_Cache)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(arc_cache src/ARC_Cache.cpp)
add_executable(opt_cache src/optimal_cache.cpp)

target_include_directories(arc_cache PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(opt_cache PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

#target_link_libraries(arc_cache ARC_Cache.hpp)
#target_link_libraries(opt_cache optimal_cache.hpp)


if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(arc_cache PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(opt_cache PRIVATE -Wall -Wextra -Wpedantic)
endif()
