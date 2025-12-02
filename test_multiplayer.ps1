# test_multiplayer.ps1

# Réinitialiser le compteur de fenêtres dans le registre Qt
Write-Host "Reinitialisation du compteur de fenetres..." -ForegroundColor Cyan
Remove-ItemProperty -Path "HKCU:\Software\Coinche\WindowCounter" -Name "windowCount" -ErrorAction SilentlyContinue

# Lancer le serveur en arrière-plan avec console visible
Write-Host "Lancement du serveur..." -ForegroundColor Green
Start-Process -FilePath ".\build\server.exe" -WindowStyle Normal

# Attendre que le serveur démarre
Start-Sleep -Seconds 2

# Identifiants des joueurs (email, password)
$players = @(
    @{Email = "aaa@aaa.fr"; Password = "aaaaaa"},
    @{Email = "bbb@bbb.fr"; Password = "bbbbbb"},
    @{Email = "ccc@ccc.fr"; Password = "cccccc"},
    @{Email = "ddd@ddd.fr"; Password = "dddddd"}
)

# Lancer les 4 clients avec auto-login (le positionnement est automatique via WindowPositioner)
foreach ($player in $players) {
    Write-Host "Lancement du client: $($player.Email)" -ForegroundColor Cyan
    Start-Process -FilePath ".\build\coinche.exe" -ArgumentList "--email", $player.Email, "--password", $player.Password -WindowStyle Normal
    Start-Sleep -Milliseconds 800
}

Write-Host "`nTous les clients sont lances et positionnes!" -ForegroundColor Green
Write-Host "Appuyez sur une touche pour tout fermer..." -ForegroundColor Yellow
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

# Fermer tous les processus
Write-Host "`nFermeture des processus..." -ForegroundColor Yellow
Get-Process coinche* -ErrorAction SilentlyContinue | Stop-Process -Force
Get-Process server* -ErrorAction SilentlyContinue | Stop-Process -Force
Write-Host "Tous les processus sont fermes." -ForegroundColor Red