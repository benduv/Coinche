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
gtest_discover_tests(coinche_tests DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests fonctionnels CAPOT et GENERALE
# ========================================
add_executable(test_capot_generale
    capot_generale_test.cpp
)

target_include_directories(test_capot_generale PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_capot_generale PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
)

# Découvrir automatiquement les tests Google Test
include(GoogleTest)
gtest_discover_tests(test_capot_generale DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests fonctionnels COINCHE et SURCOINCHE
# ========================================
add_executable(test_coinche
    coinche_test.cpp
)

target_include_directories(test_coinche PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_coinche PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
)

# Découvrir automatiquement les tests Google Test
include(GoogleTest)
gtest_discover_tests(test_coinche DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests unitaires ScoreCalculator
# ========================================
add_executable(test_scorecalculator
    scorecalculator_test.cpp
)

target_include_directories(test_scorecalculator PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_scorecalculator PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
)

include(GoogleTest)
gtest_discover_tests(test_scorecalculator DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests unitaires Belote ScoreCalculator
# ========================================
add_executable(test_belote_score
    belote_score_test.cpp
)

target_include_directories(test_belote_score PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_belote_score PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
)

include(GoogleTest)
gtest_discover_tests(test_belote_score DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests unitaires GameServer
# ========================================
add_executable(test_gameserver
    gameserver_test.cpp
)

target_include_directories(test_gameserver PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_gameserver PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
)

# Découvrir automatiquement les tests Google Test
include(GoogleTest)
gtest_discover_tests(test_gameserver DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests unitaires DatabaseManager
# ========================================
add_executable(test_databasemanager
    databasemanager_test.cpp
    ../server/DatabaseManager.cpp
)

target_include_directories(test_databasemanager PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_databasemanager PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
    Qt6::Sql
)

# Découvrir automatiquement les tests Google Test
include(GoogleTest)
gtest_discover_tests(test_databasemanager DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests unitaires NetworkManager
# ========================================
add_executable(test_networkmanager
    networkmanager_test.cpp
)

target_include_directories(test_networkmanager PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_networkmanager PRIVATE
    gtest_main
    Qt6::Core
)

# Découvrir automatiquement les tests Google Test
include(GoogleTest)
gtest_discover_tests(test_networkmanager DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests d'intégration NetworkManager
# ========================================
add_executable(test_networkmanager_integration
    networkmanager_integration_test.cpp
    ../server/NetworkManager.cpp
)

target_include_directories(test_networkmanager_integration PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_networkmanager_integration PRIVATE
    gtest_main
    coinche_client
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Qml
    Qt6::Test
)

# Découvrir automatiquement les tests Google Test
include(GoogleTest)
gtest_discover_tests(test_networkmanager_integration DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests d'intégration GameServer
# ========================================
add_executable(test_gameserver_integration
    gameserver_integration_test.cpp
    ../server/GameServer.cpp
    ../server/DatabaseManager.cpp
    ../server/SmtpClient.cpp
    ../server/StatsReporter.cpp
)

target_include_directories(test_gameserver_integration PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_gameserver_integration PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
    Qt6::Test
)

# Découvrir automatiquement les tests Google Test
include(GoogleTest)
gtest_discover_tests(test_gameserver_integration DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests unitaires système d'amis (DatabaseManager)
# ========================================
add_executable(test_friends
    friends_test.cpp
    ../server/DatabaseManager.cpp
)

target_include_directories(test_friends PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_friends PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
    Qt6::Sql
)

include(GoogleTest)
gtest_discover_tests(test_friends DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests d'intégration système d'amis (WebSocket)
# ========================================
add_executable(test_friends_integration
    friends_integration_test.cpp
    ../server/GameServer.cpp
    ../server/DatabaseManager.cpp
    ../server/SmtpClient.cpp
    ../server/StatsReporter.cpp
)

target_include_directories(test_friends_integration PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_friends_integration PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
    Qt6::Test
)

include(GoogleTest)
gtest_discover_tests(test_friends_integration DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests unitaires mode Belote
# ========================================
add_executable(test_belote_mode
    belote_mode_test.cpp
)

target_include_directories(test_belote_mode PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_belote_mode PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
)

include(GoogleTest)
gtest_discover_tests(test_belote_mode DISCOVERY_MODE PRE_TEST)

# ========================================
# Tests d'intégration lobby privé (2 et 4 joueurs)
# ========================================
add_executable(test_private_lobby_integration
    private_lobby_integration_test.cpp
    ../server/GameServer.cpp
    ../server/DatabaseManager.cpp
    ../server/SmtpClient.cpp
    ../server/StatsReporter.cpp
)

target_include_directories(test_private_lobby_integration PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/server
)

target_link_libraries(test_private_lobby_integration PRIVATE
    gtest_main
    coinche_common
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
    Qt6::Sql
    Qt6::Test
)

include(GoogleTest)
gtest_discover_tests(test_private_lobby_integration DISCOVERY_MODE PRE_TEST)