# Script pour télécharger OpenSSL 3.x pour Android
# Compatible avec Qt 6.9.3 et nécessaire pour les connexions WSS

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Téléchargement d'OpenSSL pour Android" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# Version OpenSSL à télécharger (OpenSSL 3.0)
$OPENSSL_VERSION = "3.0.14"
$BASE_URL = "https://github.com/KDAB/android_openssl/raw/master/ssl_3"

# Architectures à télécharger
$ARCHITECTURES = @(
    @{Name = "arm64-v8a"; Files = @("libssl_3_arm64-v8a.so", "libcrypto_3_arm64-v8a.so")},
    @{Name = "armeabi-v7a"; Files = @("libssl_3_armeabi-v7a.so", "libcrypto_3_armeabi-v7a.so")},
    @{Name = "x86_64"; Files = @("libssl_3_x86_64.so", "libcrypto_3_x86_64.so")}
)

foreach ($arch in $ARCHITECTURES) {
    $archName = $arch.Name
    $targetDir = "android\libs\$archName"

    Write-Host "Architecture: $archName" -ForegroundColor Yellow

    # Créer le dossier si nécessaire
    if (!(Test-Path $targetDir)) {
        New-Item -ItemType Directory -Path $targetDir -Force | Out-Null
    }

    # Télécharger chaque fichier
    foreach ($fileName in $arch.Files) {
        $url = "$BASE_URL/$fileName"

        # Nom de fichier standardisé (sans le suffixe d'architecture)
        if ($fileName -like "*ssl_3*") {
            $outputName = "libssl_3.so"
        } else {
            $outputName = "libcrypto_3.so"
        }

        $outputPath = "$targetDir\$outputName"

        Write-Host "  Téléchargement: $fileName..." -NoNewline

        try {
            # Télécharger avec Invoke-WebRequest
            Invoke-WebRequest -Uri $url -OutFile $outputPath -ErrorAction Stop

            # Vérifier la taille du fichier
            $fileSize = (Get-Item $outputPath).Length / 1MB
            Write-Host " OK ($([math]::Round($fileSize, 2)) MB)" -ForegroundColor Green
        } catch {
            Write-Host " ERREUR" -ForegroundColor Red
            Write-Host "    Impossible de télécharger: $url" -ForegroundColor Red
            Write-Host "    Erreur: $_" -ForegroundColor Red
        }
    }

    Write-Host ""
}

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Téléchargement terminé!" -ForegroundColor Green
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Les bibliothèques OpenSSL ont été téléchargées dans:" -ForegroundColor White
Write-Host "  - android\libs\arm64-v8a\" -ForegroundColor Gray
Write-Host "  - android\libs\armeabi-v7a\" -ForegroundColor Gray
Write-Host "  - android\libs\x86_64\" -ForegroundColor Gray
Write-Host ""
Write-Host "Vous pouvez maintenant recompiler l'APK Android." -ForegroundColor White
Write-Host "Les bibliothèques seront automatiquement incluses dans l'APK." -ForegroundColor White
