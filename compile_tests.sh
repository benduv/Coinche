# Compilation des tests
g++ -std=c++17 -I. \
    -I"$(dirname $(which g++))/../include" \
    tests/carte_test.cpp \
    tests/deck_test.cpp \
    tests/player_test.cpp \
    tests/gamemanager_test.cpp \
    Carte.cpp \
    Deck.cpp \
    Player.cpp \
    GameManager.cpp \
    -o tests/run_tests \
    -L"$(dirname $(which g++))/../lib" \
    -lgtest -lgtest_main -pthread

# Exécution des tests
./tests/run_tests