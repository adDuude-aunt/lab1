cmake_minimum_required(VERSION 3.24)
project(WarehouseSimulator LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(CTest)
include(FetchContent)
FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
FetchContent_MakeAvailable(googletest)
add_executable(warehouse_tests
    testcoordinate.cpp
    coordinate.cpp
    battery.cpp
    testbattery.cpp
    testcell.cpp
    cell.cpp
    bot.cpp
    testbot.cpp
    order.cpp
    testorder.cpp
)
target_compile_features(warehouse_tests PRIVATE cxx_std_23)
target_link_libraries(warehouse_tests PRIVATE GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(warehouse_tests DISCOVERY_MODE PRE_TEST)