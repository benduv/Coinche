cmake_minimum_required(VERSION 3.14)
project(Coinche)

# Compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Create library target
add_library(
    coinche_lib
    Carte.cpp
    Deck.cpp
    Player.cpp
    GameManager.cpp
)

# Create main executable
add_executable(coinche main.cpp)
target_link_libraries(coinche PRIVATE coinche_lib)

# Add tests subdirectory
add_subdirectory(tests)