# setup_android_sdk.ps1
# Script pour guider l'installation du SDK/NDK Android

Write-Host "=== Configuration Android SDK/NDK pour Qt ===" -ForegroundColor Cyan
Write-Host ""

# Vérifier si Android Studio est installé
$androidStudioPaths = @(
    "$Env:LOCALAPPDATA\Android\Sdk",
    "$Env:USERPROFILE\AppData\Local\Android\Sdk",
    "C:\Android\Sdk"
)

$sdkPath = $null
foreach ($path in $androidStudioPaths) {
    if (Test-Path $path) {
        $sdkPath = $path
        Write-Host "✓ Android SDK trouvé: $sdkPath" -ForegroundColor Green
        break
    }
}

if ($null -eq $sdkPath) {
    Write-Host "✗ Android SDK non trouvé" -ForegroundColor Red
    Write-Host ""
    Write-Host "ÉTAPE 1 : Installer Android Studio" -ForegroundColor Yellow
    Write-Host "  1. Télécharger depuis: https://developer.android.com/studio" -ForegroundColor White
    Write-Host "  2. Installer Android Studio" -ForegroundColor White
    Write-Host "  3. Lancer Android Studio" -ForegroundColor White
    Write-Host "  4. Suivre le setup wizard" -ForegroundColor White
    Write-Host ""
    Write-Host "ÉTAPE 2 : Installer SDK Tools" -ForegroundColor Yellow
    Write-Host "  1. Dans Android Studio: Tools → SDK Manager" -ForegroundColor White
    Write-Host "  2. Onglet 'SDK Platforms': Installer Android 14.0 (API 34)" -ForegroundColor White
    Write-Host "  3. Onglet 'SDK Tools': Installer:" -ForegroundColor White
    Write-Host "     - Android SDK Build-Tools 34.0.0" -ForegroundColor White
    Write-Host "     - NDK (Side by side) version 26.1.10909125" -ForegroundColor White
    Write-Host "     - CMake" -ForegroundColor White
    Write-Host "     - Android Emulator" -ForegroundColor White
    Write-Host ""

    $response = Read-Host "Voulez-vous ouvrir la page de téléchargement Android Studio? (O/N)"
    if ($response -eq "O" -or $response -eq "o") {
        Start-Process "https://developer.android.com/studio"
    }

    Write-Host ""
    Write-Host "Une fois Android Studio installé, relancez ce script." -ForegroundColor Cyan
    exit 0
}

Write-Host ""

# Vérifier les composants installés
$buildTools = Get-ChildItem -Path "$sdkPath\build-tools" -ErrorAction SilentlyContinue
if ($buildTools) {
    Write-Host "✓ Build Tools trouvés:" -ForegroundColor Green
    $buildTools | Select-Object -First 3 | ForEach-Object { Write-Host "  - $($_.Name)" -ForegroundColor White }
} else {
    Write-Host "✗ Build Tools non trouvés" -ForegroundColor Red
}

$ndkVersions = Get-ChildItem -Path "$sdkPath\ndk" -ErrorAction SilentlyContinue
if ($ndkVersions) {
    Write-Host "✓ NDK trouvé:" -ForegroundColor Green
    $ndkVersions | Select-Object -First 3 | ForEach-Object { Write-Host "  - $($_.Name)" -ForegroundColor White }

    $targetNdk = $ndkVersions | Where-Object { $_.Name -like "26.*" } | Select-Object -First 1
    if ($targetNdk) {
        Write-Host "✓ NDK 26.x trouvé: $($targetNdk.Name)" -ForegroundColor Green
    } else {
        Write-Host "⚠ NDK 26.x recommandé non trouvé" -ForegroundColor Yellow
    }
} else {
    Write-Host "✗ NDK non trouvé" -ForegroundColor Red
}

$platforms = Get-ChildItem -Path "$sdkPath\platforms" -ErrorAction SilentlyContinue
if ($platforms) {
    Write-Host "✓ Platforms trouvées:" -ForegroundColor Green
    $platforms | Select-Object -Last 3 | ForEach-Object { Write-Host "  - $($_.Name)" -ForegroundColor White }

    $api34 = $platforms | Where-Object { $_.Name -eq "android-34" }
    if ($api34) {
        Write-Host "✓ API 34 (Android 14) trouvé" -ForegroundColor Green
    } else {
        Write-Host "⚠ API 34 recommandé non trouvé" -ForegroundColor Yellow
    }
} else {
    Write-Host "✗ Platforms non trouvées" -ForegroundColor Red
}

Write-Host ""

# Vérifier JDK
$jdkPaths = @(
    "$Env:JAVA_HOME",
    "$Env:ProgramFiles\Eclipse Adoptium\jdk-17*",
    "$Env:ProgramFiles\Java\jdk-17*",
    "C:\Program Files\Java\jdk-17*"
)

$jdkPath = $null
foreach ($path in $jdkPaths) {
    if ($path -and (Test-Path $path)) {
        $jdkPath = $path
        break
    }
}

if ($jdkPath) {
    Write-Host "✓ JDK trouvé: $jdkPath" -ForegroundColor Green
} else {
    Write-Host "✗ JDK 17 non trouvé" -ForegroundColor Red
    Write-Host "  Télécharger depuis: https://adoptium.net/temurin/releases/?version=17" -ForegroundColor White
}

Write-Host ""
Write-Host "=== Configuration Qt Creator ===" -ForegroundColor Cyan
Write-Host ""
Write-Host "Prochaines étapes dans Qt Creator:" -ForegroundColor Yellow
Write-Host "  1. Ouvrir Qt Creator" -ForegroundColor White
Write-Host "  2. Edit → Preferences (ou Tools → Options)" -ForegroundColor White
Write-Host "  3. Devices → Android" -ForegroundColor White
Write-Host "  4. Configurer:" -ForegroundColor White
Write-Host "     - JDK location: $jdkPath" -ForegroundColor White
Write-Host "     - Android SDK location: $sdkPath" -ForegroundColor White
Write-Host "     - Android NDK list: Sélectionner une version 26.x" -ForegroundColor White
Write-Host "  5. Cliquer 'Apply'" -ForegroundColor White
Write-Host "  6. Vérifier que tout est coché en vert ✓" -ForegroundColor White
Write-Host ""

# Sauvegarder les chemins dans un fichier
$configFile = "android_paths.txt"
@"
Android SDK: $sdkPath
JDK: $jdkPath
Qt Android: C:\Qt\6.9.3\android_arm64_v8a

À configurer dans Qt Creator → Preferences → Devices → Android
"@ | Out-File -FilePath $configFile -Encoding UTF8

Write-Host "Chemins sauvegardés dans: $configFile" -ForegroundColor Green
Write-Host ""
Write-Host "Appuyez sur une touche pour continuer..." -ForegroundColor Yellow
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
