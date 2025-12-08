# fix_android_sdk_path.ps1
# Script pour vérifier et afficher le bon chemin SDK

Write-Host "=== Verification chemin Android SDK ===" -ForegroundColor Cyan
Write-Host ""

$correctPath = "$Env:LOCALAPPDATA\Android\Sdk"

if (Test-Path $correctPath) {
    Write-Host "Chemin SDK correct trouve:" -ForegroundColor Green
    Write-Host $correctPath -ForegroundColor White
    Write-Host ""
    Write-Host "COPIEZ ce chemin (Ctrl+C):" -ForegroundColor Yellow
    Write-Host $correctPath -ForegroundColor Cyan
    Write-Host ""
    Write-Host "Ensuite dans Android Studio:" -ForegroundColor Yellow
    Write-Host "1. File -> Settings (ou Ctrl+Alt+S)" -ForegroundColor White
    Write-Host "2. Appearance & Behavior -> System Settings -> Android SDK" -ForegroundColor White
    Write-Host "3. Dans 'Android SDK Location', COLLEZ ce chemin" -ForegroundColor White
    Write-Host "4. Cliquez Apply" -ForegroundColor White
    Write-Host "5. L'onglet SDK Tools devrait se debloquer" -ForegroundColor White

    # Copier dans le presse-papier
    Set-Clipboard -Value $correctPath
    Write-Host ""
    Write-Host "Chemin copie dans le presse-papier!" -ForegroundColor Green
    Write-Host "Vous pouvez directement faire Ctrl+V dans Android Studio" -ForegroundColor Green
} else {
    Write-Host "ERREUR: SDK non trouve a l'emplacement attendu" -ForegroundColor Red
    Write-Host "Cherchons ailleurs..." -ForegroundColor Yellow

    $alternatePaths = @(
        "C:\Android\Sdk",
        "$Env:USERPROFILE\Android\Sdk",
        "C:\Users\$Env:USERNAME\Android\Sdk"
    )

    foreach ($path in $alternatePaths) {
        if (Test-Path $path) {
            Write-Host "SDK trouve ici: $path" -ForegroundColor Green
            Set-Clipboard -Value $path
            break
        }
    }
}

Write-Host ""
Write-Host "Appuyez sur une touche..." -ForegroundColor Yellow
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
