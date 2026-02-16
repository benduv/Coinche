#!/bin/bash

# Script pour compiler et exécuter les tests Coinche
# Usage: ./run_tests.sh [test_name]
#   test_name: gameserver, capot, coinche, databasemanager, coinche_tests (carte/deck/player), all (default: all)

# Couleurs pour l'affichage
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

TEST_TYPE=${1:-all}
BUILD_DIR="../build"

echo -e "${BLUE}==================================${NC}"
echo -e "${BLUE}  Tests Coinche${NC}"
echo -e "${BLUE}==================================${NC}"
echo ""

# Créer le dossier build si nécessaire
if [ ! -d "$BUILD_DIR" ]; then
    echo -e "${BLUE}Création du dossier build...${NC}"
    mkdir -p "$BUILD_DIR"
fi

# Aller dans le dossier build
cd "$BUILD_DIR"

# Compiler
echo -e "${BLUE}Compilation...${NC}"
if [ "$TEST_TYPE" == "all" ]; then
    cmake --build . --target coinche_tests test_gameserver test_capot_generale test_coinche test_databasemanager test_networkmanager test_networkmanager_integration test_gameserver_integration
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erreur de compilation${NC}"
        echo ""
        echo "Appuyez sur Entrée pour fermer..."
        read
        exit 1
    fi
else
    cmake --build . --target "test_$TEST_TYPE"
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erreur de compilation${NC}"
        echo ""
        echo "Appuyez sur Entrée pour fermer..."
        read
        exit 1
    fi
fi

echo ""
echo -e "${GREEN}✓ Compilation réussie${NC}"
echo ""

# Exécuter les tests
cd tests

# Compteurs pour le résumé
TOTAL_FAILED=0

run_test() {
    local test_name=$1
    local test_label=$2

    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
    echo -e "${BLUE}  $test_label${NC}"
    echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"

    ./$test_name
    if [ $? -eq 0 ]; then
        echo -e "${GREEN}✓ $test_label passés${NC}"
    else
        echo -e "${RED}✗ $test_label échoués${NC}"
        TOTAL_FAILED=$((TOTAL_FAILED + 1))
    fi
    echo ""
}

case $TEST_TYPE in
    gameserver)
        echo -e "${BLUE}Exécution des tests GameServer...${NC}"
        ./test_gameserver
        ;;
    capot)
        echo -e "${BLUE}Exécution des tests CAPOT/GENERALE...${NC}"
        ./test_capot_generale
        ;;
    coinche)
        echo -e "${BLUE}Exécution des tests COINCHE/SURCOINCHE...${NC}"
        ./test_coinche
        ;;
    databasemanager)
        echo -e "${BLUE}Exécution des tests DatabaseManager...${NC}"
        ./test_databasemanager
        ;;
    networkmanager)
        echo -e "${BLUE}Exécution des tests NetworkManager...${NC}"
        ./test_networkmanager
        ;;
    networkmanager_integration)
        echo -e "${BLUE}Exécution des tests d'intégration NetworkManager...${NC}"
        ./test_networkmanager_integration
        ;;
    gameserver_integration)
        echo -e "${BLUE}Exécution des tests d'intégration GameServer...${NC}"
        ./test_gameserver_integration
        ;;
    coinche_tests)
        echo -e "${BLUE}Exécution des tests Carte/Deck/Player...${NC}"
        ./coinche_tests
        ;;
    all)
        echo -e "${BLUE}Exécution de TOUS les tests...${NC}"
        echo ""

        run_test "coinche_tests" "Tests Carte/Deck/Player"
        run_test "test_gameserver" "Tests GameServer"
        run_test "test_capot_generale" "Tests CAPOT/GENERALE"
        run_test "test_coinche" "Tests COINCHE/SURCOINCHE"
        run_test "test_databasemanager" "Tests DatabaseManager"
        run_test "test_networkmanager" "Tests NetworkManager"
        run_test "test_networkmanager_integration" "Tests Intégration NetworkManager"
        run_test "test_gameserver_integration" "Tests Intégration GameServer"
        ;;
    *)
        echo -e "${RED}Type de test invalide: $TEST_TYPE${NC}"
        echo "Usage: ./run_tests.sh [gameserver|capot|coinche|databasemanager|networkmanager|networkmanager_integration|gameserver_integration|coinche_tests|all]"
        echo ""
        echo "Appuyez sur Entrée pour fermer..."
        read
        exit 1
        ;;
esac

echo ""
if [ $TOTAL_FAILED -eq 0 ]; then
    echo -e "${GREEN}==================================${NC}"
    echo -e "${GREEN}  Tous les tests ont réussi ! ✓${NC}"
    echo -e "${GREEN}==================================${NC}"
else
    echo -e "${RED}==================================${NC}"
    echo -e "${RED}  $TOTAL_FAILED suite(s) de tests en échec${NC}"
    echo -e "${RED}==================================${NC}"
fi

echo ""
echo "Appuyez sur Entrée pour fermer..."
read
