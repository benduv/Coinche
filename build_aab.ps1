# Script PowerShell pour creer un AAB multi-architecture
# A partir de deux builds Qt Creator existants

# Chemins des builds existants
$arm64Build = "build\Qt_6_11_0_pour_Android_arm64_v8a-Release\android-build-coinche"
$armv7Build = "build\Qt_6_11_0_pour_Android_armeabi_v7a-Release\android-build-coinche"

# Dossier de build combine
$combinedBuild = "build\Android_Combined_Release\android-build-coinche"

Write-Host "=== Creation d'un AAB multi-architecture ===" -ForegroundColor Cyan

# Creer le dossier de build combine en copiant arm64 comme base
if (Test-Path $combinedBuild) {
    Write-Host "Suppression de l'ancien build combine..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $combinedBuild
}

Write-Host "Copie du build arm64-v8a comme base..." -ForegroundColor Green
Copy-Item -Recurse $arm64Build $combinedBuild

# Copier les libs armeabi-v7a
Write-Host "Ajout des bibliotheques armeabi-v7a..." -ForegroundColor Green
$armv7Libs = "$armv7Build\libs\armeabi-v7a"
$targetLibs = "$combinedBuild\libs\armeabi-v7a"

if (-not (Test-Path $armv7Libs)) {
    Write-Host "ERREUR: $armv7Libs introuvable!" -ForegroundColor Red
    exit 1
}

Copy-Item -Recurse "$armv7Libs\*" $targetLibs -Force
Write-Host "Bibliotheques armeabi-v7a copiees" -ForegroundColor Green

# Remplacer libcoinche par la version RelWithDebInfo (contient les symboles DWARF)
# Gradle avec debugSymbolLevel 'FULL' va extraire les symboles puis stripper le .so
Write-Host "Remplacement de libcoinche par la version RelWithDebInfo..." -ForegroundColor Green
$arm64RelDbg = "build\Qt_6_11_0_pour_Android_arm64_v8a-RelWithDebInfo\libcoinche_arm64-v8a.so"
$armv7RelDbg = "build\Qt_6_11_0_pour_Android_armeabi_v7a-RelWithDebInfo\libcoinche_armeabi-v7a.so"
if (Test-Path $arm64RelDbg) {
    Copy-Item $arm64RelDbg "$combinedBuild\libs\arm64-v8a\libcoinche_arm64-v8a.so" -Force
    Write-Host "  arm64-v8a: OK" -ForegroundColor Green
} else {
    Write-Host "  arm64-v8a: introuvable, symboles manquants" -ForegroundColor Yellow
}
if (Test-Path $armv7RelDbg) {
    Copy-Item $armv7RelDbg "$combinedBuild\libs\armeabi-v7a\libcoinche_armeabi-v7a.so" -Force
    Write-Host "  armeabi-v7a: OK" -ForegroundColor Green
} else {
    Write-Host "  armeabi-v7a: introuvable, symboles manquants" -ForegroundColor Yellow
}

# Modifier build.gradle pour forcer les deux architectures et targetSdk 36
Write-Host "Configuration de build.gradle pour multi-architecture..." -ForegroundColor Green
$buildGradle = "$combinedBuild\build.gradle"
(Get-Content $buildGradle) `
    -replace 'abiFilters qtTargetAbiList\.split\(","\)', 'abiFilters "arm64-v8a", "armeabi-v7a"' `
    -replace 'versionCode 1', 'versionCode 12' `
    -replace "versionName '1.0.0'", "versionName '0.2.2'" |
    Set-Content $buildGradle

# Verifier que les deux architectures sont presentes
Write-Host ""
Write-Host "Architectures presentes:" -ForegroundColor Cyan
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
$armv7So  = "build\Qt_6_11_0_pour_Android_armeabi_v7a-RelWithDebInfo\libcoinche_armeabi-v7a.so"
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

if (Test-Path $armv7So) {
    New-Item -ItemType Directory -Force "$tmpDir\armeabi-v7a" | Out-Null
    Copy-Item $armv7So "$tmpDir\armeabi-v7a\libcoinche_armeabi-v7a.so"
    Write-Host "  armeabi-v7a: OK" -ForegroundColor Green
    $hasSymbols = $true
} else {
    Write-Host "  armeabi-v7a: introuvable ($armv7So)" -ForegroundColor Yellow
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
