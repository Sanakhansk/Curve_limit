```cmake
cmake_minimum_required(VERSION 3.10)
project(curve_limit_agent)
set(CMAKE_CXX_STANDARD 17)
find_package(Threads REQUIRED)
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
add_executable(agent
    src/main.cpp
    src/ethereum_rpc.cpp
    src/curve_pool.cpp
    src/order_agent.cpp
)
target_include_directories(agent PRIVATE src)
target_link_libraries(agent PRIVATE ${CURL_LIBRARIES} Threads::Threads)
```