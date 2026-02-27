# Script PowerShell pour creer un AAB multi-architecture
# A partir de deux builds Qt Creator existants

# Chemins des builds existants
$arm64Build = "build\Android_Qt_6_9_3_Clang_arm64_v8a-arm64-v8a_Release\android-build-coinche"
$armv7Build = "build\Android_Qt_6_9_3_Clang_armeabi_v7a-armeabi-v7_Release\android-build-coinche"

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

# Modifier build.gradle pour forcer les deux architectures et targetSdk 35
Write-Host "Configuration de build.gradle pour multi-architecture et targetSdk 35..." -ForegroundColor Green
$buildGradle = "$combinedBuild\build.gradle"
(Get-Content $buildGradle) `
    -replace 'abiFilters qtTargetAbiList\.split\(","\)', 'abiFilters "arm64-v8a", "armeabi-v7a"' `
    -replace 'compileSdkVersion 34', 'compileSdkVersion 35' `
    -replace 'targetSdkVersion = 34', 'targetSdkVersion = 35' `
    -replace "buildToolsVersion '34\.0\.0'", "buildToolsVersion '35.0.0'" `
    -replace 'versionCode 1', 'versionCode 2' |
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
