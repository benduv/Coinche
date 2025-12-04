# update_test_avatars.ps1
# Script pour mettre à jour les avatars des comptes de test dans la base de données

Write-Host "Mise a jour des avatars des comptes de test..." -ForegroundColor Cyan

# Chemin vers la base de données
$dbPath = ".\coinche.db"

if (-Not (Test-Path $dbPath)) {
    Write-Host "Erreur: Base de donnees non trouvee a $dbPath" -ForegroundColor Red
    exit 1
}

# Vérifier si sqlite3 est disponible (d'abord dans le répertoire courant, puis dans le PATH)
$sqlite3Command = $null

if (Test-Path ".\sqlite3.exe") {
    Write-Host "Utilisation de sqlite3.exe dans le repertoire courant" -ForegroundColor Green
    $sqlite3Command = ".\sqlite3.exe"
} else {
    $sqlite3FromPath = Get-Command sqlite3 -ErrorAction SilentlyContinue
    if ($null -ne $sqlite3FromPath) {
        Write-Host "Utilisation de sqlite3 depuis le PATH" -ForegroundColor Green
        $sqlite3Command = "sqlite3"
    }
}

if ($null -eq $sqlite3Command) {
    Write-Host "Erreur: sqlite3.exe non trouve" -ForegroundColor Red
    Write-Host "  - Placez sqlite3.exe dans le repertoire courant, OU" -ForegroundColor Yellow
    Write-Host "  - Installez-le dans le PATH depuis: https://www.sqlite.org/download.html" -ForegroundColor Yellow
    exit 1
}

# Exécuter les requêtes SQL directement
Write-Host "Mise a jour de l'avatar pour aaa@aaa.fr -> avataaars1.svg" -ForegroundColor Green
& $sqlite3Command $dbPath "UPDATE users SET avatar = 'avataaars1.svg' WHERE email = 'aaa@aaa.fr';"

Write-Host "Mise a jour de l'avatar pour bbb@bbb.fr -> avataaars2.svg" -ForegroundColor Green
& $sqlite3Command $dbPath "UPDATE users SET avatar = 'avataaars2.svg' WHERE email = 'bbb@bbb.fr';"

Write-Host "Mise a jour de l'avatar pour ccc@ccc.fr -> avataaars3.svg" -ForegroundColor Green
& $sqlite3Command $dbPath "UPDATE users SET avatar = 'avataaars3.svg' WHERE email = 'ccc@ccc.fr';"

Write-Host "Mise a jour de l'avatar pour ddd@ddd.fr -> avataaars4.svg" -ForegroundColor Green
& $sqlite3Command $dbPath "UPDATE users SET avatar = 'avataaars4.svg' WHERE email = 'ddd@ddd.fr';"

Write-Host "`nVerification des avatars mis a jour:" -ForegroundColor Cyan
& $sqlite3Command $dbPath "SELECT pseudo, email, avatar FROM users WHERE email IN ('aaa@aaa.fr', 'bbb@bbb.fr', 'ccc@ccc.fr', 'ddd@ddd.fr');"

Write-Host "`nMise a jour terminee!" -ForegroundColor Green
