# Script pour télécharger OpenSSL 3.x pour Android (version alternative)
# Utilise le dépôt GitHub avec téléchargement direct

Write-Host "======================================" -ForegroundColor Cyan
Write-Host "Téléchargement d'OpenSSL pour Android" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""

# URLs alternatives - directement depuis les releases GitHub
$BASE_URL = "https://github.com/KDAB/android_openssl/blob/master/ssl_3"

Write-Host "MÉTHODE MANUELLE REQUISE:" -ForegroundColor Yellow
Write-Host ""
Write-Host "1. Visitez ce dépôt GitHub:" -ForegroundColor White
Write-Host "   https://github.com/KDAB/android_openssl/tree/master/ssl_3" -ForegroundColor Cyan
Write-Host ""
Write-Host "2. Téléchargez les fichiers suivants:" -ForegroundColor White
Write-Host ""
Write-Host "   Pour arm64-v8a (smartphone moderne):" -ForegroundColor Yellow
Write-Host "   - libssl_3_arm64-v8a.so" -ForegroundColor Gray
Write-Host "   - libcrypto_3_arm64-v8a.so" -ForegroundColor Gray
Write-Host ""
Write-Host "   Pour armeabi-v7a (smartphone ancien):" -ForegroundColor Yellow
Write-Host "   - libssl_3_armeabi-v7a.so" -ForegroundColor Gray
Write-Host "   - libcrypto_3_armeabi-v7a.so" -ForegroundColor Gray
Write-Host ""
Write-Host "   Pour x86_64 (émulateur Android):" -ForegroundColor Yellow
Write-Host "   - libssl_3_x86_64.so" -ForegroundColor Gray
Write-Host "   - libcrypto_3_x86_64.so" -ForegroundColor Gray
Write-Host ""
Write-Host "3. Placez les fichiers dans les dossiers suivants:" -ForegroundColor White
Write-Host ""
Write-Host "   android\libs\arm64-v8a\" -ForegroundColor Cyan
Write-Host "     -> libssl_3_arm64-v8a.so (renommer en libssl_3.so)" -ForegroundColor Gray
Write-Host "     -> libcrypto_3_arm64-v8a.so (renommer en libcrypto_3.so)" -ForegroundColor Gray
Write-Host ""
Write-Host "   android\libs\armeabi-v7a\" -ForegroundColor Cyan
Write-Host "     -> libssl_3_armeabi-v7a.so (renommer en libssl_3.so)" -ForegroundColor Gray
Write-Host "     -> libcrypto_3_armeabi-v7a.so (renommer en libcrypto_3.so)" -ForegroundColor Gray
Write-Host ""
Write-Host "   android\libs\x86_64\" -ForegroundColor Cyan
Write-Host "     -> libssl_3_x86_64.so (renommer en libssl_3.so)" -ForegroundColor Gray
Write-Host "     -> libcrypto_3_x86_64.so (renommer en libcrypto_3.so)" -ForegroundColor Gray
Write-Host ""
Write-Host "======================================" -ForegroundColor Cyan
Write-Host ""
Write-Host "Alternative: Télécharger automatiquement avec Git LFS" -ForegroundColor Yellow
Write-Host ""
Write-Host "Si vous avez Git installé avec LFS:" -ForegroundColor White
Write-Host "  git clone https://github.com/KDAB/android_openssl.git temp_openssl" -ForegroundColor Gray
Write-Host "  cd temp_openssl" -ForegroundColor Gray
Write-Host "  git lfs pull" -ForegroundColor Gray
Write-Host ""
Write-Host "Puis copiez les fichiers depuis temp_openssl\ssl_3\" -ForegroundColor White
