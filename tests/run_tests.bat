@echo off
REM Script pour compiler et exécuter les tests Coinche sur Windows
REM Usage: run_tests.bat [test_name]
REM   test_name: gameserver, capot, coinche, all (default: all)

setlocal

set TEST_TYPE=%1
if "%TEST_TYPE%"=="" set TEST_TYPE=all
set BUILD_DIR=..\build

echo ==================================
echo   Tests Coinche - GameServer
echo ==================================
echo.

REM Créer le dossier build si nécessaire
if not exist "%BUILD_DIR%" (
    echo Creation du dossier build...
    mkdir "%BUILD_DIR%"
)

REM Aller dans le dossier build
cd "%BUILD_DIR%"

REM Compiler
echo Compilation...
if "%TEST_TYPE%"=="all" (
    cmake --build . --target test_gameserver test_capot_generale test_coinche
) else (
    cmake --build . --target test_%TEST_TYPE%
)

if errorlevel 1 (
    echo Erreur de compilation
    exit /b 1
)

echo.
echo Compilation reussie
echo.

REM Exécuter les tests
cd tests

if "%TEST_TYPE%"=="gameserver" (
    echo Execution des tests GameServer...
    test_gameserver.exe
) else if "%TEST_TYPE%"=="capot" (
    echo Execution des tests CAPOT/GENERALE...
    test_capot_generale.exe
) else if "%TEST_TYPE%"=="coinche" (
    echo Execution des tests COINCHE/SURCOINCHE...
    test_coinche.exe
) else if "%TEST_TYPE%"=="all" (
    echo Execution de TOUS les tests...
    echo.
    echo ================================
    echo   Tests GameServer
    echo ================================
    test_gameserver.exe
    echo.
    echo ================================
    echo   Tests CAPOT/GENERALE
    echo ================================
    test_capot_generale.exe
    echo.
    echo ================================
    echo   Tests COINCHE/SURCOINCHE
    echo ================================
    test_coinche.exe
) else (
    echo Type de test invalide: %TEST_TYPE%
    echo Usage: run_tests.bat [gameserver^|capot^|coinche^|all]
    exit /b 1
)

echo.
echo ==================================
echo   Tous les tests ont reussi !
echo ==================================

endlocal
