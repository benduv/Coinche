# install_jdk.ps1
# Script pour télécharger et installer JDK 17

Write-Host "=== Installation JDK 17 ===" -ForegroundColor Cyan
Write-Host ""

# Vérifier si JDK déjà installé
$jdkPaths = @(
    "C:\Program Files\Eclipse Adoptium\jdk-17*",
    "C:\Program Files\Java\jdk-17*",
    "C:\Program Files\Microsoft\jdk-17*"
)

$existingJdk = $null
foreach ($pattern in $jdkPaths) {
    $found = Get-Item $pattern -ErrorAction SilentlyContinue | Select-Object -First 1
    if ($found) {
        $existingJdk = $found.FullName
        break
    }
}

if ($existingJdk) {
    Write-Host "JDK 17 deja installe: $existingJdk" -ForegroundColor Green
    Write-Host ""
    Write-Host "Configuration de JAVA_HOME..." -ForegroundColor Yellow
    [System.Environment]::SetEnvironmentVariable("JAVA_HOME", $existingJdk, "User")
    Write-Host "JAVA_HOME defini: $existingJdk" -ForegroundColor Green
    Write-Host ""
    Write-Host "Redemarrez Qt Creator pour appliquer les changements." -ForegroundColor Yellow
    exit 0
}

Write-Host "JDK 17 non trouve. Installation necessaire." -ForegroundColor Yellow
Write-Host ""
Write-Host "Option 1: Telechargement automatique depuis Adoptium" -ForegroundColor Cyan
Write-Host "Option 2: Telechargement manuel" -ForegroundColor Cyan
Write-Host ""

$choice = Read-Host "Choisir option (1 ou 2)"

if ($choice -eq "1") {
    Write-Host ""
    Write-Host "Ouverture du site de telechargement..." -ForegroundColor Yellow
    Start-Process "https://adoptium.net/temurin/releases/?version=17&os=windows&arch=x64&package=jdk"
    Write-Host ""
    Write-Host "Instructions:" -ForegroundColor Cyan
    Write-Host "1. Sur la page qui s'ouvre, cliquer sur le bouton .msi" -ForegroundColor White
    Write-Host "2. Lancer le fichier telecharge" -ForegroundColor White
    Write-Host "3. Suivre l'installation (tout par defaut)" -ForegroundColor White
    Write-Host "4. Cocher 'Set JAVA_HOME' si propose" -ForegroundColor White
    Write-Host "5. Relancer ce script apres installation" -ForegroundColor White
} else {
    Write-Host ""
    Write-Host "Instructions installation manuelle:" -ForegroundColor Cyan
    Write-Host "1. Aller sur: https://adoptium.net/temurin/releases/" -ForegroundColor White
    Write-Host "2. Choisir:" -ForegroundColor White
    Write-Host "   - Version: 17" -ForegroundColor White
    Write-Host "   - OS: Windows" -ForegroundColor White
    Write-Host "   - Architecture: x64" -ForegroundColor White
    Write-Host "   - Package: JDK" -ForegroundColor White
    Write-Host "3. Telecharger le .msi" -ForegroundColor White
    Write-Host "4. Installer" -ForegroundColor White
    Write-Host "5. Relancer ce script" -ForegroundColor White
}

Write-Host ""
Write-Host "Appuyez sur une touche pour fermer..." -ForegroundColor Yellow
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
