# Étapes Finales dans Android Studio

Vous avez déjà l'Android SDK installé ✅. Il reste juste 2 outils à ajouter.

## 📍 Étape 1 : Ouvrir SDK Manager

1. Lancer **Android Studio**
2. Sur l'écran d'accueil :
   - Cliquer sur **⋮ (3 points verticaux)** en haut à droite
   - Sélectionner **SDK Manager**

   OU si un projet est ouvert :
   - Menu **Tools → SDK Manager**

## 🔧 Étape 2 : Installer NDK

Dans la fenêtre SDK Manager :

1. Cliquer sur l'onglet **"SDK Tools"** (en haut)

2. Dans la liste, **cocher** :
   ```
   [ ] Android SDK Build-Tools (déjà coché ✓)
   [ ] Android SDK Platform-Tools (déjà coché ✓)
   [✓] NDK (Side by side)          ◄── COCHER CELUI-CI
   [ ] CMake                        ◄── Recommandé aussi
   ```

3. Cliquer sur **"Apply"** (en bas à droite)

4. Une fenêtre "Confirm Change" apparaît :
   - Vérifier que ça affiche "NDK"
   - Cliquer **"OK"**

5. **Attendre le téléchargement** (~500-800 MB)
   - Barre de progression visible
   - Prend 3-10 minutes selon connexion

6. Cliquer **"Finish"** quand terminé

7. **Noter la version installée** :
   - Regarder dans la colonne "Version"
   - Exemple : `27.2.12479018` ou `26.1.10909125`
   - **Notez ce numéro**, vous en aurez besoin pour Qt Creator

## ☕ Étape 3 : Installer/Vérifier JDK

### Option A : Via Android Studio (Recommandé)

Dans SDK Manager, même fenêtre, onglet **"SDK Tools"** :

1. Chercher dans la liste :
   ```
   [✓] Android SDK Command-line Tools
   [ ] JetBrains Runtime         ◄── Ou cherchez "JDK"
   ```

2. Si vous voyez **"JDK"** ou **"JetBrains Runtime"**, cochez-le

3. Apply → OK → Attendre

### Option B : Installation Manuelle (Si Option A pas dispo)

Si vous ne voyez pas de JDK dans Android Studio :

1. **Fermer Android Studio**

2. **Télécharger JDK 17** :
   - Aller sur : https://adoptium.net/temurin/releases/
   - Filtres :
     - Version: **17**
     - Operating System: **Windows**
     - Architecture: **x64**
     - Package Type: **JDK**
   - Cliquer sur le bouton **.msi** pour télécharger

3. **Installer** :
   - Double-cliquer sur le fichier `.msi` téléchargé
   - Suivre l'installation (tout par défaut)
   - ⚠️ **Important** : Cocher "Set JAVA_HOME variable" si proposé
   - Terminer l'installation

4. **Vérifier** :
   Ouvrir PowerShell et taper :
   ```powershell
   java -version
   ```

   Devrait afficher :
   ```
   openjdk version "17.0.x"
   ```

## 📝 Étape 4 : Noter les Chemins

Vous aurez besoin de ces chemins pour Qt Creator :

### Android SDK
```
C:\Users\33672\AppData\Local\Android\Sdk
```

### Android NDK
```
C:\Users\33672\AppData\Local\Android\Sdk\ndk\[VERSION]
```
Remplacer `[VERSION]` par la version installée (ex: `27.2.12479018`)

Pour trouver la version exacte :
```powershell
ls C:\Users\33672\AppData\Local\Android\Sdk\ndk\
```

### JDK (Option B seulement)
```
C:\Program Files\Eclipse Adoptium\jdk-17.0.x-hotspot
```

Ou si via Android Studio :
```
C:\Program Files\Android\Android Studio\jbr
```

## ✅ Vérification Rapide

Ouvrir PowerShell et lancer :

```powershell
powershell -ExecutionPolicy Bypass -File check_android_setup.ps1
```

Tous les éléments doivent être en **vert** ✅

---

## ➡️ Prochaine Étape

Une fois les outils installés, passer à la configuration dans Qt Creator :
- Voir : **CONFIGURATION_QT_CREATOR.md**

---

## 💾 Checklist Installation

- [✅] Android SDK installé
- [✅] Build Tools installé
- [✅] Platform API installé
- [ ] **NDK installé** ◄── À FAIRE
- [ ] **JDK 17 installé** ◄── À FAIRE
- [ ] Qt Creator configuré ◄── Ensuite

---

## ❓ Besoin d'Aide ?

Si vous rencontrez un problème :
1. Vérifier que vous avez suffisamment d'espace disque (5+ GB libres)
2. Redémarrer Android Studio
3. Vérifier votre connexion internet
4. Dans Android Studio : File → Invalidate Caches → Invalidate and Restart
