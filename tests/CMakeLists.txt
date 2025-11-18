cmake_minimum_required(VERSION 3.14)
project(CoincheTesting)

# Fetch GoogleTest
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        release-1.11.0
)
FetchContent_MakeAvailable(googletest)

qt_add_resources(RESOURCES resources.qrc)

enable_testing()

add_executable(
    coinche_tests
    carte_test.cpp
    deck_test.cpp
    player_test.cpp
)

target_link_libraries(
    coinche_tests
    gtest_main
    coinche_common
    Qt6::Core
)

include(GoogleTest)
gtest_discover_tests(coinche_tests)

# ========================================
# Tests fonctionnels CAPOT et GENERALE
# ========================================
add_executable(test_capot_generale
    test_capot_generale.cpp
    GameServerTest.h
)

target_include_directories(test_capot_generale PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_capot_generale PRIVATE
    coinche_common
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
)