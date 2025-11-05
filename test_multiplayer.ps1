# test_multiplayer.ps1

# Lancer le serveur en arrière-plan
Write-Host "Lancement du serveur..." -ForegroundColor Green
Start-Process -FilePath ".\build\server.exe" -WindowStyle Normal

# Attendre que le serveur démarre
Start-Sleep -Seconds 2

# Lancer 4 clients avec des noms différents
$playerNames = @("Alice", "Bob", "Charlie", "David")

foreach ($name in $playerNames) {
    Write-Host "Lancement du client: $name" -ForegroundColor Cyan
    Start-Process -FilePath ".\build\coinche.exe" -ArgumentList "--name", $name -WindowStyle Normal
    Start-Sleep -Milliseconds 500
}

Write-Host "`nTous les clients sont lancés!" -ForegroundColor Green
Write-Host "Appuyez sur une touche pour tout fermer..." -ForegroundColor Yellow
$null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")

# Fermer tous les processus
Get-Process coinche* | Stop-Process -Force
Write-Host "Tous les processus sont fermés." -ForegroundColor Red