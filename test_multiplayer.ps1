# test_multiplayer.ps1

# Réinitialiser le compteur de fenêtres dans le registre Qt
Write-Host "Reinitialisation du compteur de fenetres..." -ForegroundColor Cyan
Remove-ItemProperty -Path "HKCU:\Software\Coinche\WindowCounter" -Name "windowCount" -ErrorAction SilentlyContinue

# Mettre à jour les avatars des comptes de test dans la base de données
Write-Host "Mise a jour des avatars des comptes de test..." -ForegroundColor Cyan
if (Test-Path ".\update_test_avatars.ps1") {
    & .\update_test_avatars.ps1
} else {
    Write-Host "Attention: Script update_test_avatars.ps1 non trouve" -ForegroundColor Yellow
}

# Lancer le serveur en arrière-plan avec console visible
Write-Host "`nLancement du serveur..." -ForegroundColor Green
Start-Process -FilePath ".\build\Desktop_Qt_6_9_3_MinGW_64_bit-Debug\server.exe" -WindowStyle Normal

# Attendre que le serveur démarre
Start-Sleep -Seconds 2

# Identifiants des joueurs (email, password, avatar)
$players = @(
    @{Email = "aaa@aaa.fr"; Password = "aaaaaa"; Avatar = "avataaars1.svg"},
    @{Email = "bbb@bbb.fr"; Password = "bbbbbb"; Avatar = "avataaars2.svg"},
    @{Email = "ccc@ccc.fr"; Password = "cccccc"; Avatar = "avataaars3.svg"},
    @{Email = "ddd@ddd.fr"; Password = "dddddd"; Avatar = "avataaars4.svg"}
)

# Lancer les 4 clients avec auto-login (le positionnement est automatique via WindowPositioner)
# Les avatars sont récupérés automatiquement depuis la base de données lors du login
# Utiliser --no-autologin pour empêcher l'auto-login depuis QSettings (qui partage les credentials entre instances)
foreach ($player in $players) {
    Write-Host "Lancement du client: $($player.Email) (avatar: $($player.Avatar))" -ForegroundColor Cyan
    Start-Process -FilePath ".\build\Desktop_Qt_6_9_3_MinGW_64_bit-Debug\coinche.exe" -ArgumentList "--email", $player.Email, "--password", $player.Password, "--no-autologin" -WindowStyle Normal
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