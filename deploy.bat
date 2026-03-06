@echo off
setlocal EnableDelayedExpansion

echo ============================================
echo  Coinche - Script de deploiement Windows
echo ============================================

:: ---- Chemins a adapter si necessaire ----
set QT_BIN=C:\Qt\6.9.3\mingw_64\bin
set MINGW_BIN=C:\Qt\Tools\mingw1310_64\bin
set "INNO_COMPILER=C:\Program Files (x86)\Inno Setup 6\iscc.exe"

:: Exe release produit par Qt Creator
set RELEASE_EXE=build\Desktop_Qt_6_9_3_MinGW_64_bit-Release\coinche.exe

:: Dossier de sortie intermediaire (contenu de l'installateur)
set DEPLOY_DIR=installer\deploy

:: ---- Verification de l'exe release ----
if not exist "%RELEASE_EXE%" (
    echo [ERREUR] Exe release introuvable : %RELEASE_EXE%
    echo Compilez d'abord en Release dans Qt Creator.
    pause
    exit /b 1
)

:: ---- Nettoyage et preparation du dossier deploy ----
echo.
echo [1/4] Preparation du dossier de deploiement...
if exist "%DEPLOY_DIR%" rmdir /s /q "%DEPLOY_DIR%"
mkdir "%DEPLOY_DIR%"
copy "%RELEASE_EXE%" "%DEPLOY_DIR%\coinche.exe" >nul
echo      coinche.exe copie.

:: ---- windeployqt ----
echo.
echo [2/4] Execution de windeployqt (copie des libs Qt)...
"%QT_BIN%\windeployqt.exe" --release --qmldir qml "%DEPLOY_DIR%\coinche.exe"
if errorlevel 1 (
    echo [ERREUR] windeployqt a echoue.
    pause
    exit /b 1
)
echo      Libs Qt copiees.

:: ---- DLLs runtime MinGW ----
echo.
echo [3/4] Copie des DLLs runtime MinGW...
copy "%MINGW_BIN%\libgcc_s_seh-1.dll"  "%DEPLOY_DIR%\" >nul
copy "%MINGW_BIN%\libstdc++-6.dll"     "%DEPLOY_DIR%\" >nul
copy "%MINGW_BIN%\libwinpthread-1.dll" "%DEPLOY_DIR%\" >nul
echo      DLLs MinGW copiees.

:: ---- Inno Setup ----
echo.
echo [4/4] Creation de l'installateur...
if not exist "%INNO_COMPILER%" goto :no_inno

"%INNO_COMPILER%" installer\coinche.iss
if errorlevel 1 (
    echo [ERREUR] Inno Setup a echoue.
    pause
    exit /b 1
)
echo      Installateur cree dans installer\output\
goto :done

:no_inno
echo [AVERTISSEMENT] Inno Setup non trouve.
echo Installez Inno Setup depuis https://jrsoftware.org/isdl.php
echo Le dossier installer\deploy\ est pret pour une distribution en ZIP.

:done
echo.
echo ============================================
echo  Deploiement termine !
echo ============================================
pause
