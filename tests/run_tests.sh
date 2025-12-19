#!/bin/bash

# Script pour compiler et exécuter les tests Coinche
# Usage: ./run_tests.sh [test_name]
#   test_name: gameserver, capot, coinche, all (default: all)

set -e  # Arrêter en cas d'erreur

# Couleurs pour l'affichage
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

TEST_TYPE=${1:-all}
BUILD_DIR="../build"

echo -e "${BLUE}==================================${NC}"
echo -e "${BLUE}  Tests Coinche - GameServer${NC}"
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
    cmake --build . --target test_gameserver test_capot_generale test_coinche
else
    cmake --build . --target "test_$TEST_TYPE"
fi

echo ""
echo -e "${GREEN}✓ Compilation réussie${NC}"
echo ""

# Exécuter les tests
cd tests

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
    all)
        echo -e "${BLUE}Exécution de TOUS les tests...${NC}"
        echo ""
        echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        echo -e "${BLUE}  Tests GameServer${NC}"
        echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        ./test_gameserver
        echo ""
        echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        echo -e "${BLUE}  Tests CAPOT/GENERALE${NC}"
        echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        ./test_capot_generale
        echo ""
        echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        echo -e "${BLUE}  Tests COINCHE/SURCOINCHE${NC}"
        echo -e "${BLUE}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${NC}"
        ./test_coinche
        ;;
    *)
        echo -e "${RED}Type de test invalide: $TEST_TYPE${NC}"
        echo "Usage: ./run_tests.sh [gameserver|capot|coinche|all]"
        exit 1
        ;;
esac

echo ""
echo -e "${GREEN}==================================${NC}"
echo -e "${GREEN}  Tous les tests ont réussi ! ✓${NC}"
echo -e "${GREEN}==================================${NC}"
