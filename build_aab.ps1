# Script PowerShell pour creer un AAB arm64-v8a uniquement
# A partir d'un build Qt Creator existant

# Chemin du build existant
$arm64Build = "build\Qt_6_11_0_pour_Android_arm64_v8a-Release\android-build-coinche"

# Dossier de build combine
$combinedBuild = "build\Android_Combined_Release\android-build-coinche"

Write-Host "=== Creation d'un AAB arm64-v8a ===" -ForegroundColor Cyan

# Creer le dossier de build combine en copiant arm64 comme base
if (Test-Path $combinedBuild) {
    Write-Host "Suppression de l'ancien build combine..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $combinedBuild
}

Write-Host "Copie du build arm64-v8a..." -ForegroundColor Green
Copy-Item -Recurse $arm64Build $combinedBuild

# Remplacer libcoinche par la version RelWithDebInfo (contient les symboles DWARF)
# Gradle avec debugSymbolLevel 'FULL' va extraire les symboles puis stripper le .so
Write-Host "Remplacement de libcoinche par la version RelWithDebInfo..." -ForegroundColor Green
$arm64RelDbg = "build\Qt_6_11_0_pour_Android_arm64_v8a-RelWithDebInfo\libcoinche_arm64-v8a.so"
if (Test-Path $arm64RelDbg) {
    Copy-Item $arm64RelDbg "$combinedBuild\libs\arm64-v8a\libcoinche_arm64-v8a.so" -Force
    Write-Host "  arm64-v8a: OK" -ForegroundColor Green
} else {
    Write-Host "  arm64-v8a: introuvable, symboles manquants" -ForegroundColor Yellow
}

# Modifier build.gradle pour forcer arm64-v8a uniquement et targetSdk 36
Write-Host "Configuration de build.gradle..." -ForegroundColor Green
$buildGradle = "$combinedBuild\build.gradle"
(Get-Content $buildGradle) `
    -replace 'abiFilters qtTargetAbiList\.split\(","\)', 'abiFilters "arm64-v8a"' `
    -replace 'versionCode 1', 'versionCode 13' `
    -replace "versionName '1.0.0'", "versionName '0.2.3'" |
    Set-Content $buildGradle

# Supprimer armeabi-v7a si present
$armv7Dir = "$combinedBuild\libs\armeabi-v7a"
if (Test-Path $armv7Dir) {
    Remove-Item -Recurse -Force $armv7Dir
    Write-Host "Suppression de armeabi-v7a" -ForegroundColor Yellow
}

# Verifier l'architecture presente
Write-Host ""
Write-Host "Architecture presente:" -ForegroundColor Cyan
Get-ChildItem "$combinedBuild\libs" -Directory | ForEach-Object {
    Write-Host "  - $($_.Name)" -ForegroundColor White
}

# Lancer la creation de l'AAB
Write-Host ""
Write-Host "=== Creation de l'AAB ===" -ForegroundColor Cyan
Set-Location $combinedBuild

Write-Host "Execution de: gradlew bundleRelease..." -ForegroundColor Yellow
& .\gradlew.bat bundleRelease

$buildExitCode = $LASTEXITCODE

Set-Location $PSScriptRoot

if ($buildExitCode -ne 0) {
    Write-Host ""
    Write-Host "Erreur lors de la creation de l'AAB" -ForegroundColor Red
    Write-Host "Code de retour: $buildExitCode" -ForegroundColor Red
    exit 1
}

Write-Host ""
Write-Host "AAB cree avec succes!" -ForegroundColor Green
$aabFile = "$combinedBuild\build\outputs\bundle\release\coinche-release.aab"
if (Test-Path $aabFile) {
    $fullPath = Resolve-Path $aabFile
    Write-Host ""
    Write-Host "Fichier AAB: $fullPath" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Pour verifier les architectures dans l'AAB:" -ForegroundColor Yellow
    Write-Host "  bundletool build-apks --bundle=`"$fullPath`" --output=test.apks" -ForegroundColor Gray
}

# Generer le fichier native-debug-symbols.zip pour Google Play
Write-Host ""
Write-Host "=== Generation des symboles natifs pour Google Play ===" -ForegroundColor Cyan

$arm64So  = "build\Qt_6_11_0_pour_Android_arm64_v8a-RelWithDebInfo\libcoinche_arm64-v8a.so"
$symbolsZip = "build\coinche-native-debug-symbols.zip"
$tmpDir = "build\symbols_tmp"

# Nettoyer le dossier temporaire
if (Test-Path $tmpDir) { Remove-Item -Recurse -Force $tmpDir }

$hasSymbols = $false

if (Test-Path $arm64So) {
    New-Item -ItemType Directory -Force "$tmpDir\arm64-v8a" | Out-Null
    Copy-Item $arm64So "$tmpDir\arm64-v8a\libcoinche_arm64-v8a.so"
    Write-Host "  arm64-v8a: OK" -ForegroundColor Green
    $hasSymbols = $true
} else {
    Write-Host "  arm64-v8a: introuvable ($arm64So)" -ForegroundColor Yellow
}

if ($hasSymbols) {
    if (Test-Path $symbolsZip) { Remove-Item -Force $symbolsZip }
    Compress-Archive -Path "$tmpDir\*" -DestinationPath $symbolsZip
    Remove-Item -Recurse -Force $tmpDir
    $zipPath = Resolve-Path $symbolsZip
    $zipSize = [Math]::Round((Get-Item $symbolsZip).Length / 1MB, 1)
    Write-Host ""
    Write-Host "Symbols ZIP ($($zipSize) MB): $zipPath" -ForegroundColor Cyan
    Write-Host "A uploader sur la Play Console : Version > Modifier > Symboles de debogage natifs" -ForegroundColor Gray
} else {
    Write-Host "Aucun .so trouve, zip non genere." -ForegroundColor Red
}
