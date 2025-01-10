cmake_minimum_required(VERSION 3.10)
project(ThreadPoolProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add executable
add_executable(thread_pool_test
    main.cpp
    ThreadPool.h
)

# Find and link threads library
find_package(Threads REQUIRED)
target_link_libraries(thread_pool_test PRIVATE Threads::Threads) 