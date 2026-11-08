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

# Библиотека домена
add_library(warehouse_domain
    coordinate.cpp
    battery.cpp
    cell.cpp
    map.cpp
    bot.cpp
    order.cpp
    moverules.cpp
    pathes.cpp
    journal.cpp
    dispatcher.cpp
    simulator.cpp
    graphs.cpp
)
target_compile_features(warehouse_domain PUBLIC cxx_std_23)

# Тесты
add_executable(warehouse_tests
    testcoordinate.cpp
    testbattery.cpp
    testcell.cpp
    testmap.cpp
    testbot.cpp
    testorder.cpp
    testmoverules.cpp
    testpath.cpp
    testjournal.cpp
    testdispatcher.cpp
    testsimulator.cpp
    testgraphs.cpp
)
target_link_libraries(warehouse_tests PRIVATE warehouse_domain GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(warehouse_tests DISCOVERY_MODE PRE_TEST)

# Основное приложение
add_executable(warehouse_sim main.cpp)
target_link_libraries(warehouse_sim PRIVATE warehouse_domain)