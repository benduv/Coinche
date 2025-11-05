cmake_minimum_required(VERSION 3.14)
project(Coinche)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Qml WebSockets)

# Compiler flags
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Create library target
add_library(
    coinche_lib
    Carte.cpp
    Deck.cpp
    Player.cpp
    GameManager.cpp
    HandModel.cpp
    GameModel.cpp
)
target_include_directories(coinche_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Qt6Core_INCLUDE_DIRS}
    ${Qt6Quick_INCLUDE_DIRS}
    ${Qt6QuickControls2_INCLUDE_DIRS}
    ${Qt6Qml_INCLUDE_DIRS}
    ${Qt6WebSockets_INCLUDE_DIRS}
)
target_link_libraries(coinche_lib PUBLIC
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Qml
    Qt6::WebSockets
)

target_sources(coinche_lib PRIVATE
    GameServer.h
    NetworkManager.h
)

# Create main executable
qt_add_executable(coinche 
    main.cpp
    resources.qrc
)

#add_executable(server server_main.cpp)
#target_link_libraries(server PRIVATE Qt6::Core Qt6::Network Qt6::WebSockets)

qt_add_resources(RESOURCES resources.qrc)
target_sources(coinche PRIVATE ${RESOURCES})

# Add QML modules
qt_add_qml_module(coinche
    URI Coinche
    VERSION 1.0
    QML_FILES
        qml/MainMenu.qml
    RESOURCE_PREFIX "/"
)

target_link_libraries(coinche PRIVATE 
    coinche_lib
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Qml
    Qt6::WebSockets
)

# Créer l'exécutable serveur
add_executable(server
    server_main.cpp
    GameServer.h
    NetworkManager.h
)

# Lier les bibliothèques nécessaires au serveur
target_link_libraries(server PRIVATE
    coinche_lib
    Qt6::Core
    Qt6::Network
    Qt6::WebSockets
)

# Add tests subdirectory
add_subdirectory(tests)