# Script pour ajouter une règle de pare-feu pour le serveur Coinche
# Exécutez ce script en tant qu'administrateur

Write-Host "Ajout de la règle de pare-feu pour le port 1234..." -ForegroundColor Green

# Supprimer la règle si elle existe déjà
Remove-NetFirewallRule -DisplayName "Coinche Server TCP 1234" -ErrorAction SilentlyContinue

# Ajouter la règle pour TCP
New-NetFirewallRule -DisplayName "Coinche Server TCP 1234" `
                    -Direction Inbound `
                    -Protocol TCP `
                    -LocalPort 1234 `
                    -Action Allow `
                    -Profile Any

Write-Host "Règle de pare-feu ajoutée avec succès!" -ForegroundColor Green
Write-Host "Le serveur Coinche peut maintenant accepter les connexions sur le port 1234" -ForegroundColor Cyan
