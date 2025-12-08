# Installation JDK 17 - Guide Rapide

Le JDK (Java Development Kit) est nécessaire pour compiler les applications Android avec Qt.

## 📥 Téléchargement

### Option 1 : Lien Direct (Plus Rapide)

Cliquez sur ce lien pour télécharger directement le JDK 17 pour Windows :

**👉 https://adoptium.net/temurin/releases/?version=17**

Sur la page qui s'ouvre :
1. Les filtres devraient être automatiquement :
   - **Version**: 17 - LTS
   - **Operating System**: Windows
   - **Architecture**: x64
   - **Package Type**: JDK

2. Cliquez sur le gros bouton bleu **".msi"** pour télécharger
   - Taille : ~190 MB
   - Fichier : `OpenJDK17U-jdk_x64_windows_hotspot_17.x.x.msi`

### Option 2 : Téléchargement Manuel

Si le lien ne fonctionne pas :
1. Aller sur : https://adoptium.net/
2. Cliquer sur "Temurin 17 (LTS)"
3. Sélectionner Windows, x64, JDK
4. Télécharger le .msi

## 📦 Installation

1. **Lancer le fichier .msi** téléchargé

2. **Écran de bienvenue** :
   - Cliquer "Next"

3. **Licence** :
   - Accepter la licence
   - Cliquer "Next"

4. **Options d'installation** (Important !) :

   Vous verrez plusieurs options avec icônes :

   ✅ **Cocher obligatoirement** :
   - ✅ **Set JAVA_HOME variable** (ou "JavaHome env variable")
   - ✅ **Add to PATH** (ou "PATH env variable")

   Autres options (optionnelles) :
   - IcedTea-Web → Pas nécessaire
   - Associate .jar → Optionnel
   - Set JAVA_HOME for Eclipse → Optionnel

5. **Destination** :
   - Laisser par défaut : `C:\Program Files\Eclipse Adoptium\jdk-17.x.x-hotspot\`
   - Cliquer "Next"

6. **Installation** :
   - Cliquer "Install"
   - Accepter les permissions admin (UAC)
   - Attendre 2-3 minutes

7. **Fin** :
   - Cliquer "Finish"

## ✅ Vérification

### Vérifier l'installation

Ouvrir **PowerShell** (ou Invite de commandes) et taper :

```powershell
java -version
```

**Résultat attendu** :
```
openjdk version "17.0.x" 2024-xx-xx
OpenJDK Runtime Environment Temurin-17.0.x+x (build 17.0.x+x)
OpenJDK 64-Bit Server VM Temurin-17.0.x+x (build 17.0.x+x, mixed mode, sharing)
```

### Vérifier JAVA_HOME

```powershell
echo $Env:JAVA_HOME
```

**Résultat attendu** :
```
C:\Program Files\Eclipse Adoptium\jdk-17.0.x-hotspot
```

### Si JAVA_HOME n'est pas défini

Si la commande ci-dessus ne retourne rien, définir manuellement :

```powershell
# Trouver le chemin exact
ls "C:\Program Files\Eclipse Adoptium\"

# Copier le chemin affiché (ex: jdk-17.0.13-hotspot)
# Puis exécuter (remplacer X.X.XX par votre version):
[System.Environment]::SetEnvironmentVariable("JAVA_HOME", "C:\Program Files\Eclipse Adoptium\jdk-17.0.13-hotspot", "User")
```

**Puis redémarrer PowerShell** et vérifier à nouveau.

## 🔄 Script de Vérification Automatique

Une fois installé, lancer :

```powershell
powershell -ExecutionPolicy Bypass -File check_android_setup.ps1
```

Le JDK devrait apparaître en **vert** ✅

## ⚠️ Problèmes Courants

### "java n'est pas reconnu..."

→ JAVA_HOME ou PATH pas configuré correctement
→ Redémarrer PowerShell / Qt Creator
→ Définir manuellement JAVA_HOME (voir ci-dessus)

### "Cannot find JDK" dans Qt Creator

→ Redémarrer Qt Creator après installation
→ Vérifier dans Qt Creator : Edit → Preferences → Devices → Android
→ Le chemin JDK doit être : `C:\Program Files\Eclipse Adoptium\jdk-17.x.x-hotspot`

### Version incorrecte affichée

Si `java -version` affiche une autre version (8, 11, etc.) :
→ Un autre JDK est installé et prioritaire dans le PATH
→ Pas grave, Qt Creator utilisera le bon chemin si configuré manuellement

## ➡️ Prochaines Étapes

Une fois JDK installé et vérifié :

1. ✅ NDK devrait être installé (en cours)
2. ✅ JDK 17 installé (ce guide)
3. ⏭️ Configurer Qt Creator avec les chemins SDK/NDK/JDK
4. ⏭️ Compiler votre première APK !

---

## 📝 Chemins à Noter

Après installation, notez ces chemins pour Qt Creator :

```
Android SDK:
C:\Users\33672\AppData\Local\Android\Sdk

Android NDK:
C:\Users\33672\AppData\Local\Android\Sdk\ndk\[VERSION]

JDK 17:
C:\Program Files\Eclipse Adoptium\jdk-17.0.x-hotspot
```

Ces chemins seront nécessaires dans **Qt Creator → Preferences → Android**.

---

## 🆘 Besoin d'Aide ?

Si problème d'installation :
1. Vérifier que vous avez les droits administrateur
2. Désinstaller d'éventuels anciens JDK conflictuels
3. Redémarrer Windows après installation
4. Télécharger à nouveau le .msi si fichier corrompu
