cmake_minimum_required(VERSION 3.10)
project(ARC_Cache)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Поиск исходных файлов
file(GLOB SRC_FILES "src/*.cpp")
file(GLOB TEST_FILES "tests/*.cpp")

# Создание основной библиотеки
add_library(arc_cache ${SRC_FILES})

# Указание директории с заголовками для библиотеки
target_include_directories(arc_cache PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Создание тестового исполняемого файла
add_executable(arc_cache_test ${TEST_FILES})

# Связывание тестов с библиотекой
target_link_libraries(arc_cache_test arc_cache)

# Указание директории с заголовками для тестов
target_include_directories(arc_cache_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Добавление флагов компиляции
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(arc_cache PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(arc_cache_test PRIVATE -Wall -Wextra -Wpedantic)
endif()
