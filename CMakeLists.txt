cmake_minimum_required(VERSION 3.14)
project(Coinche VERSION 1.0.0)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Quick QuickControls2 Qml WebSockets Network Sql Multimedia)

# Compiler flags
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Configuration Android
if(ANDROID)
    #set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android" CACHE INTERNAL "")
    set(ANDROID_MIN_SDK_VERSION 28)
    set(ANDROID_TARGET_SDK_VERSION 34)
    set(ANDROID_VERSION_CODE 1)
    set(ANDROID_VERSION_NAME "1.0.0")

    # Architectures Android supportées
    if(NOT ANDROID_ABI)
        set(ANDROID_ABI "arm64-v8a")
    endif()

    # Configuration OpenSSL pour Android (nécessaire pour WSS)
    # Utiliser qt_add_executable avec MANUAL_FINALIZATION pour Qt 6.5+
    set(ANDROID_EXTRA_LIBS
        "${CMAKE_CURRENT_SOURCE_DIR}/android/libs/${ANDROID_ABI}/libssl_3.so"
        "${CMAKE_CURRENT_SOURCE_DIR}/android/libs/${ANDROID_ABI}/libcrypto_3.so"
        CACHE INTERNAL ""
    )
endif()

# ========================================
# Bibliothèque commune (logique de jeu)
# ========================================
add_library(coinche_common STATIC
    Carte.cpp
    Carte.h
    Deck.cpp
    Deck.h
    Player.cpp
    Player.h
)

target_include_directories(coinche_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(coinche_common PUBLIC
    Qt6::Core
)

# ========================================
# Bibliothèque client (GameModel + HandModel)
# ========================================
add_library(coinche_client STATIC
    HandModel.cpp
    HandModel.h
    GameModel.cpp
    GameModel.h
    # NetworkManager est header-only, on le met directement dans l'exe client
)

target_include_directories(coinche_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(coinche_client PUBLIC
    coinche_common
    Qt6::Core
    Qt6::Qml
    Qt6::WebSockets
    Qt6::Network
)

# ========================================
# Exécutable CLIENT (coinche)
# ========================================
qt_add_executable(coinche
    main.cpp
    server/NetworkManager.h  # Header-only, mis directement dans le client
    WindowPositioner.h       # Header-only pour positionner les fenêtres
    OrientationHelper.h      # Header-only pour contrôler l'orientation Android via JNI
    resources.qrc
)

qt_add_resources(RESOURCES resources.qrc)
target_sources(coinche PRIVATE ${RESOURCES})

# Add QML modules
qt_add_qml_module(coinche
    URI Coinche
    VERSION 1.0
    QML_FILES
        qml/MainMenu.qml
        qml/PrivateLobbyView.qml
        qml/LobbyRoomView.qml
        # Ajoutez tous vos fichiers QML ici
    RESOURCE_PREFIX "/"
)

#QML_FILES qml/AvatarSelector.qml qml/Card.qml qml/CoincheView.qml qml/GameOverPopup.qml qml/LoginView.qml qml/MatchMakingView.qml qml/StatsView.qml
target_link_libraries(coinche PRIVATE
    coinche_client  # Contient GameModel + HandModel
    Qt6::Core
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Qml
    Qt6::WebSockets
    Qt6::Network
    Qt6::Multimedia
)

# Configuration OpenSSL pour Android - Inclure les libs dans l'APK
if(ANDROID)
    # Méthode 1: Via QT_ANDROID_EXTRA_LIBS (pour Qt 6.5+)
    set_target_properties(coinche PROPERTIES
        QT_ANDROID_EXTRA_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/android/libs/${ANDROID_ABI}/libssl_3.so;${CMAKE_CURRENT_SOURCE_DIR}/android/libs/${ANDROID_ABI}/libcrypto_3.so"
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android"
    )

    # Méthode 2: Copier automatiquement les libs dans le dossier de build
    # Ceci garantit qu'elles seront incluses dans l'APK
    add_custom_command(TARGET coinche POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/android/libs/${ANDROID_ABI}/libssl_3.so"
            "${CMAKE_BINARY_DIR}/android-build-coinche/libs/${ANDROID_ABI}/libssl_3.so"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/android/libs/${ANDROID_ABI}/libcrypto_3.so"
            "${CMAKE_BINARY_DIR}/android-build-coinche/libs/${ANDROID_ABI}/libcrypto_3.so"
        COMMENT "Copie des bibliothèques OpenSSL pour Android (${ANDROID_ABI})"
    )

    message(STATUS "OpenSSL Android: Bibliothèques configurées pour ${ANDROID_ABI}")
    message(STATUS "  - libssl_3.so: ${CMAKE_CURRENT_SOURCE_DIR}/android/libs/${ANDROID_ABI}/libssl_3.so")
    message(STATUS "  - libcrypto_3.so: ${CMAKE_CURRENT_SOURCE_DIR}/android/libs/${ANDROID_ABI}/libcrypto_3.so")
endif()

# ========================================
# Exécutable SERVEUR (server) - Desktop uniquement
# ========================================
if(NOT ANDROID)
    add_executable(server
        server/server_main.cpp
        server/GameServer.h  # Header-only, mis directement dans le serveur
        server/DatabaseManager.h
        server/DatabaseManager.cpp
        server/SmtpClient.h
        server/SmtpClient.cpp
        server/StatsReporter.h
        server/StatsReporter.cpp
    )

    target_include_directories(server PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
    )

    # Le serveur utilise seulement la logique commune, PAS NetworkManager
    target_link_libraries(server PRIVATE
        coinche_common  # Seulement Carte, Player, Deck
        Qt6::Core
        Qt6::Network
        Qt6::WebSockets
        Qt6::Sql
    )

    # ========================================
    # Tests - Desktop uniquement
    # ========================================
    add_subdirectory(tests)
endif()
