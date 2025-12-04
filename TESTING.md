# Guide de Test Multijoueur

Ce document explique comment tester le jeu en mode multijoueur avec 4 joueurs automatiques.

## Prérequis

1. **SQLite3** doit être installé et accessible dans le PATH
   - Téléchargement : https://www.sqlite.org/download.html
   - Sur Windows, téléchargez "sqlite-tools-win32-x86-*.zip"
   - Extrayez et ajoutez le dossier au PATH

2. Les comptes de test doivent exister dans la base de données :
   - aaa@aaa.fr (pseudo: aaa, mot de passe: aaaaaa)
   - bbb@bbb.fr (pseudo: bbb, mot de passe: bbbbbb)
   - ccc@ccc.fr (pseudo: ccc, mot de passe: cccccc)
   - ddd@ddd.fr (pseudo: ddd, mot de passe: dddddd)

## Scripts Disponibles

### `test_multiplayer.ps1`
Script principal qui lance automatiquement :
1. Mise à jour des avatars des comptes de test
2. Le serveur de jeu
3. 4 clients avec auto-connexion

**Utilisation :**
```powershell
.\test_multiplayer.ps1
```

Les fenêtres seront automatiquement positionnées en grille 2x2.

### `update_test_avatars.ps1`
Met à jour les avatars des 4 comptes de test dans la base de données :
- aaa@aaa.fr → avataaars1.svg
- bbb@bbb.fr → avataaars2.svg
- ccc@ccc.fr → avataaars3.svg
- ddd@ddd.fr → avataaars4.svg

**Utilisation :**
```powershell
.\update_test_avatars.ps1
```

### `update_test_avatars.sql`
Fichier SQL brut pour mettre à jour les avatars. Peut être exécuté manuellement :

```bash
sqlite3 coinche.db < update_test_avatars.sql
```

## Arguments de Ligne de Commande

L'application client supporte les arguments suivants :

- `--email <email>` ou `-e <email>` : Email pour auto-connexion
- `--password <password>` ou `-p <password>` : Mot de passe pour auto-connexion
- `--avatar <avatar>` ou `-a <avatar>` : Avatar à utiliser (par défaut: avataaars1.svg)
- `--name <name>` ou `-n <name>` : Nom du joueur (pour invités)

**Exemple :**
```powershell
.\build\coinche.exe --email aaa@aaa.fr --password aaaaaa
```

## Dépannage

### "sqlite3 n'est pas reconnu"
Installez SQLite3 et ajoutez-le au PATH système.

### Les avatars ne changent pas
1. Vérifiez que les fichiers SVG existent dans `resources/avatar/`
2. Exécutez manuellement `.\update_test_avatars.ps1`
3. Vérifiez la base de données :
   ```bash
   sqlite3 coinche.db "SELECT pseudo, email, avatar FROM users;"
   ```

### Les fenêtres ne se positionnent pas correctement
Le compteur de fenêtres est réinitialisé au début du script. Si le problème persiste, supprimez manuellement la clé de registre :
```powershell
Remove-ItemProperty -Path "HKCU:\Software\Coinche\WindowCounter" -Name "windowCount"
```

## Avatars Disponibles

24 avatars sont disponibles dans le dossier `resources/avatar/` :
- avataaars1.svg à avataaars24.svg

Chaque avatar est un personnage unique généré avec le style "avataaars".
