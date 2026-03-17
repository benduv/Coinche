# Script PowerShell pour generer le fichier de symboles de debug
# a uploader dans Google Play Console

$arm64Build = "build\Android_Qt_6_9_3_Clang_arm64_v8a-arm64-v8a_Release"
$armv7Build = "build\Android_Qt_6_9_3_Clang_armeabi_v7a-armeabi-v7_Release"

$outputZip = "coinche-symbols.zip"
$tempDir = "symbols_temp"

Write-Host "=== Generation des symboles de debug ===" -ForegroundColor Cyan

# Verifier que les .so existent
# .so non-strippé produit par CMake (contient les symboles de debug)
$arm64So = "$arm64Build\libcoinche_arm64-v8a.so"
$armv7So = "$armv7Build\libcoinche_armeabi-v7a.so"

if (-not (Test-Path $arm64So)) {
    Write-Host "ERREUR: $arm64So introuvable!" -ForegroundColor Red
    Write-Host "Compile d'abord le build arm64-v8a Release dans Qt Creator." -ForegroundColor Yellow
    exit 1
}

if (-not (Test-Path $armv7So)) {
    Write-Host "ERREUR: $armv7So introuvable!" -ForegroundColor Red
    Write-Host "Compile d'abord le build armeabi-v7a Release dans Qt Creator." -ForegroundColor Yellow
    exit 1
}

# Nettoyer le dossier temporaire
if (Test-Path $tempDir) {
    Remove-Item -Recurse -Force $tempDir
}

# Creer la structure attendue par Google Play:
# arm64-v8a/libcoinche_arm64-v8a.so
# armeabi-v7a/libcoinche_armeabi-v7a.so
New-Item -ItemType Directory -Path "$tempDir\arm64-v8a" | Out-Null
New-Item -ItemType Directory -Path "$tempDir\armeabi-v7a" | Out-Null

Write-Host "Copie des symboles arm64-v8a..." -ForegroundColor Green
Copy-Item $arm64So "$tempDir\arm64-v8a\libcoinche.so"

Write-Host "Copie des symboles armeabi-v7a..." -ForegroundColor Green
Copy-Item $armv7So "$tempDir\armeabi-v7a\libcoinche.so"

# Supprimer le zip precedent si existant
if (Test-Path $outputZip) {
    Remove-Item $outputZip
}

# Creer le ZIP
Write-Host "Creation du fichier ZIP..." -ForegroundColor Green
Compress-Archive -Path "$tempDir\*" -DestinationPath $outputZip

# Nettoyer le dossier temporaire
Remove-Item -Recurse -Force $tempDir

$fullPath = Resolve-Path $outputZip
Write-Host ""
Write-Host "Symboles generes avec succes!" -ForegroundColor Green
Write-Host "Fichier: $fullPath" -ForegroundColor Cyan
Write-Host ""
Write-Host "Uploader ce fichier dans Google Play Console:" -ForegroundColor Yellow
Write-Host "  Tests fermes -> Alpha -> App bundles -> (clic sur l'AAB) -> Fichiers de debogage -> Symboles natifs" -ForegroundColor Gray
