set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

cmake_minimum_required(VERSION 3.10)
project(ThreadPoolProject)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")

# Add executable
add_executable(thread_pool_test
    main.cpp
    ThreadPool.h
)

# Find and link threads library
find_package(Threads REQUIRED)
target_link_libraries(thread_pool_test PRIVATE Threads::Threads)

# Add these lines to fix the linker error
if (APPLE)
    target_link_libraries(thread_pool_test PRIVATE c++ c++abi)
endif() 