# Installation des Outils Android pour Qt

Vous avez déjà Qt avec support Android (`C:\Qt\6.9.3\android_arm64_v8a`), mais il manque les outils de build Android.

## ✅ Ce qui est déjà installé

- Qt 6.9.3 ✓
- Qt Android ARM64 ✓
- Qt Android ARMv7 ✓

## ❌ Ce qui manque

- Android SDK
- Android NDK
- JDK 17

---

## 📦 Étape 1 : Installer Android Studio (Obtenir SDK + NDK)

### Option A : Installation Complète (Recommandé si vous débutez)

1. **Télécharger Android Studio** :
   - Ouvrir : https://developer.android.com/studio
   - Cliquer sur "Download Android Studio"
   - Sauvegarder le fichier (environ 1 GB)

2. **Installer Android Studio** :
   - Lancer l'installateur
   - Suivre les étapes (installer tout par défaut)
   - ⚠️ Cocher "Android Virtual Device" pour avoir un émulateur

3. **Premier lancement** :
   - Android Studio lance un "Setup Wizard"
   - Sélectionner "Standard" installation
   - Accepter les licences
   - Attendre le téléchargement (~3-4 GB)

4. **Installer les outils nécessaires** :

   Une fois Android Studio ouvert :
   - Cliquer sur "More Actions" → "SDK Manager"
   - Ou menu "Tools" → "SDK Manager"

   **SDK Platforms (onglet)** :
   - ✅ Cocher : Android 14.0 (API 34) - Tiramisu
   - ✅ Cocher : Android 13.0 (API 33) - optionnel
   - Cliquer "Apply"

   **SDK Tools (onglet)** :
   - ✅ Android SDK Build-Tools 34.0.0 (ou plus récent)
   - ✅ NDK (Side by side) - Installer version 26.1.10909125
   - ✅ CMake
   - ✅ Android Emulator
   - ✅ Android SDK Platform-Tools
   - Cliquer "Apply"

   Attendre le téléchargement et installation (~2-3 GB)

5. **Noter le chemin d'installation** :

   Dans SDK Manager, le chemin devrait être :
   ```
   C:\Users\VotreNom\AppData\Local\Android\Sdk
   ```

---

## ☕ Étape 2 : Installer JDK 17

Qt Android nécessite Java Development Kit 17.

### Téléchargement et Installation

1. **Télécharger JDK 17** :
   - Ouvrir : https://adoptium.net/temurin/releases/?version=17
   - Sélectionner :
     - Operating System: Windows
     - Architecture: x64
     - Package Type: JDK
   - Cliquer sur le bouton de téléchargement (.msi)

2. **Installer** :
   - Lancer le fichier .msi
   - Suivre les étapes (installation par défaut)
   - Cocher "Set JAVA_HOME variable" si proposé
   - Noter le chemin (généralement `C:\Program Files\Eclipse Adoptium\jdk-17.x.x-hotspot\`)

3. **Configurer JAVA_HOME** (si pas fait automatiquement) :

   Ouvrir PowerShell en tant qu'administrateur :
   ```powershell
   [System.Environment]::SetEnvironmentVariable("JAVA_HOME", "C:\Program Files\Eclipse Adoptium\jdk-17.0.x-hotspot", "User")
   ```

4. **Vérifier** :
   ```powershell
   java -version
   ```

   Devrait afficher : `openjdk version "17.0.x"`

---

## ⚙️ Étape 3 : Configurer Qt Creator

Une fois Android Studio et JDK installés :

1. **Lancer Qt Creator**

2. **Ouvrir les Préférences** :
   - Menu : Edit → Preferences (ou Tools → Options sur Windows)

3. **Configurer Android** :
   - Section : Devices → Android

   Remplir les champs :
   ```
   JDK location:
   C:\Program Files\Eclipse Adoptium\jdk-17.0.x-hotspot

   Android SDK location:
   C:\Users\VotreNom\AppData\Local\Android\Sdk

   Android NDK list:
   [Cliquer sur "Add" et sélectionner]
   C:\Users\VotreNom\AppData\Local\Android\Sdk\ndk\26.1.10909125
   ```

4. **Appliquer** :
   - Cliquer sur "Apply"
   - Qt Creator vérifie les outils
   - Toutes les lignes doivent avoir un ✓ vert

5. **Vérifier les Kits** :
   - Section : Kits
   - Vous devriez voir apparaître :
     - Android Qt 6.9.3 ARM64-v8a
     - Android Qt 6.9.3 ARMv7

   Si non visibles :
   - Cliquer sur "Add" → "Android Device"
   - Sélectionner Qt 6.9.3 for Android ARM64-v8a

---

## 🧪 Étape 4 : Vérifier la Configuration

Dans PowerShell, lancer :

```powershell
powershell -ExecutionPolicy Bypass -File check_android_setup.ps1
```

Tous les éléments doivent être en vert ✓

---

## 🚀 Étape 5 : Tester un Build Android

1. **Ouvrir le projet Coinche** dans Qt Creator

2. **Sélectionner le Kit Android** :
   - Barre latérale gauche
   - Cliquer sur l'icône "Kit"
   - Cocher "Android Qt 6.9.3 ARM64-v8a"

3. **Build** :
   - Menu : Build → Build Project "Coinche"
   - Ou appuyer sur Ctrl+B
   - Vérifier qu'il n'y a pas d'erreurs

4. **Créer l'APK** :
   - Menu : Build → Build Android APK
   - Sélectionner "Debug" pour commencer
   - Choisir "arm64-v8a"
   - Cliquer "Create"

5. **Résultat** :
   - L'APK sera dans : `build-coinche-Android_Qt_6_9_3-Debug/android-build/build/outputs/apk/debug/`
   - Nom du fichier : `coinche-1.0.0-1-debug-arm64-v8a.apk`

---

## 🐛 Dépannage

### Erreur "SDK Build Tools not found"

Dans Android Studio SDK Manager, installer Build-Tools 34.0.0

### Erreur "NDK not configured"

Vérifier que le chemin NDK dans Qt Creator pointe vers :
`...\Android\Sdk\ndk\26.1.10909125`

### Erreur "JAVA_HOME not set"

Redémarrer Qt Creator après avoir défini JAVA_HOME

### Qt Creator ne détecte pas les kits Android

1. Fermer Qt Creator
2. Supprimer le cache : `%APPDATA%\QtProject\qtcreator\`
3. Relancer Qt Creator
4. Reconfigurer Android dans Preferences

---

## 📊 Tailles de Téléchargement

- Android Studio : ~1 GB
- SDK + NDK + Tools : ~3-4 GB
- JDK 17 : ~200 MB
- **Total : ~4-5 GB**
- **Temps estimé : 30-60 minutes** (selon connexion internet)

---

## ✅ Checklist Complète

Avant de pouvoir compiler pour Android :

- [ ] Android Studio installé
- [ ] SDK API 34 installé
- [ ] Build-Tools 34.0.0 installé
- [ ] NDK 26.1.10909125 installé
- [ ] CMake installé
- [ ] JDK 17 installé
- [ ] JAVA_HOME configuré
- [ ] Qt Creator configuré (Devices → Android)
- [ ] Kits Android visibles dans Qt Creator
- [ ] Build test réussi
- [ ] APK généré

---

## 🎯 Prochaine Étape

Une fois tous les outils installés, consultez [ANDROID_BUILD.md](ANDROID_BUILD.md) pour :
- Compiler l'application
- Tester sur émulateur
- Tester sur device physique
- Créer un APK signé

---

## 💡 Conseils

1. **Espace disque** : Assurez-vous d'avoir au moins 10 GB libres sur C:
2. **Connexion** : Téléchargements lourds, utilisez WiFi stable
3. **Temps** : Prévoyez 1h pour installer et configurer tout
4. **Patience** : Premiers builds Android sont lents (5-10 minutes)

---

## 📞 Besoin d'Aide ?

Si problèmes persistent après installation :
1. Vérifier les logs Qt Creator : "Compile Output" panel
2. Vérifier chemins dans Preferences → Android
3. Consulter : https://doc.qt.io/qt-6/android-getting-started.html
