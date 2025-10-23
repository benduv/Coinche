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

enable_testing()

add_executable(
    coinche_tests
    carte_test.cpp
    deck_test.cpp
    player_test.cpp
    gamemanager_test.cpp
)

target_link_libraries(
    coinche_tests
    gtest_main
    coinche_lib
)

include(GoogleTest)
gtest_discover_tests(coinche_tests)