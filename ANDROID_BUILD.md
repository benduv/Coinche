# Configuration Qt pour Android - Jeu de Coinche

Ce document explique comment configurer et compiler le jeu de Coinche pour Android.

## Prérequis

### 1. Installation de Qt pour Android

Vous devez installer Qt 6.5+ avec le support Android. Utilisez Qt Maintenance Tool :

1. Lancer **Qt Maintenance Tool**
2. Sélectionner **Add or remove components**
3. Cocher les composants suivants :
   - Qt 6.5.x (ou version supérieure)
   - Android ARM 64-bit (arm64-v8a)
   - Android ARMv7 (armeabi-v7a) - optionnel
   - Qt Quick
   - Qt WebSockets
   - Qt Network

### 2. Android SDK et NDK

#### Option A : Via Android Studio (Recommandé)

1. Télécharger et installer [Android Studio](https://developer.android.com/studio)
2. Lancer Android Studio
3. Aller dans **Tools → SDK Manager**
4. Dans l'onglet **SDK Platforms**, installer :
   - Android 13.0 (API 33) ou supérieur
   - Android 14.0 (API 34)
5. Dans l'onglet **SDK Tools**, installer :
   - Android SDK Build-Tools 34.0.0
   - NDK (Side by side) version 26.1.10909125
   - CMake
   - Android Emulator (pour tester)

#### Option B : Via ligne de commande

```bash
# Via sdkmanager (Windows PowerShell)
$Env:ANDROID_SDK_ROOT = "C:\Users\VotreNom\AppData\Local\Android\Sdk"
sdkmanager "platforms;android-34"
sdkmanager "build-tools;34.0.0"
sdkmanager "ndk;26.1.10909125"
sdkmanager "cmake;3.22.1"
```

### 3. Java Development Kit (JDK)

Qt pour Android nécessite JDK 17 :

1. Télécharger [JDK 17](https://adoptium.net/temurin/releases/)
2. Installer et noter le chemin d'installation
3. Définir la variable d'environnement `JAVA_HOME`

```powershell
# Windows PowerShell
[System.Environment]::SetEnvironmentVariable("JAVA_HOME", "C:\Program Files\Eclipse Adoptium\jdk-17.0.x-hotspot", "User")
```

## Configuration de Qt Creator

### 1. Configuration des Kits Android

1. Ouvrir **Qt Creator**
2. Aller dans **Edit → Preferences** (ou **Tools → Options** sur Windows)
3. Sélectionner **Devices → Android**
4. Configurer les chemins :

```
JDK location:         C:\Program Files\Eclipse Adoptium\jdk-17.0.x-hotspot
Android SDK location: C:\Users\VotreNom\AppData\Local\Android\Sdk
Android NDK list:     [Sélectionner NDK 26.1.10909125]
```

5. Cliquer sur **Apply** et vérifier que tout est coché en vert

### 2. Vérification du Kit

Dans **Kits** (même fenêtre de préférences) :
- Vérifier qu'un kit **Android Qt 6.x.x ARM64** existe
- Si non, créer un nouveau kit :
  - **Device type**: Android Device
  - **Compiler**: Android Clang (arm64-v8a)
  - **Qt version**: Qt 6.x.x for Android ARM64-v8a
  - **CMake Tool**: CMake 3.x.x

## Compilation du Projet

### Méthode 1 : Via Qt Creator (Recommandé pour débuter)

1. Ouvrir le projet dans Qt Creator (`coinche/CMakeLists.txt`)
2. Sélectionner le kit **Android Qt 6.x.x ARM64** dans la barre latérale gauche
3. Cliquer sur l'icône **Build** (marteau) ou appuyer sur `Ctrl+B`
4. Pour créer l'APK :
   - Menu **Build → Build Android APK**
   - Sélectionner **Release** ou **Debug**
   - Choisir l'architecture (arm64-v8a recommandé)
   - Cliquer sur **Create**

L'APK sera généré dans le dossier `build-coinche-Android_Qt_6_x_x-Release/android-build/build/outputs/apk/`

### Méthode 2 : Via ligne de commande

```powershell
# Configuration des variables d'environnement
$Env:ANDROID_SDK_ROOT = "C:\Users\VotreNom\AppData\Local\Android\Sdk"
$Env:ANDROID_NDK_ROOT = "$Env:ANDROID_SDK_ROOT\ndk\26.1.10909125"
$Env:JAVA_HOME = "C:\Program Files\Eclipse Adoptium\jdk-17.0.x-hotspot"
$Env:Qt6_DIR = "C:\Qt\6.5.x\android_arm64_v8a"

# Créer le dossier de build
mkdir build-android
cd build-android

# Configuration CMake pour Android
cmake .. `
    -DCMAKE_TOOLCHAIN_FILE="$Env:ANDROID_NDK_ROOT\build\cmake\android.toolchain.cmake" `
    -DANDROID_ABI=arm64-v8a `
    -DANDROID_PLATFORM=android-26 `
    -DANDROID_STL=c++_shared `
    -DQt6_DIR="$Env:Qt6_DIR\lib\cmake\Qt6" `
    -DCMAKE_BUILD_TYPE=Release

# Compilation
cmake --build . --parallel

# Génération de l'APK avec androiddeployqt
$Env:Qt6_DIR\bin\androiddeployqt.exe `
    --input android-coinche-deployment-settings.json `
    --output android-build `
    --android-platform android-34 `
    --gradle
```

## Test sur Émulateur

### 1. Créer un émulateur Android

Via Android Studio :
1. **Tools → Device Manager**
2. **Create Device**
3. Choisir un appareil (ex: Pixel 5)
4. Sélectionner une image système (API 33 ou 34, arm64-v8a)
5. Nommer l'émulateur et cliquer sur **Finish**

### 2. Lancer l'application

Dans Qt Creator :
1. Sélectionner l'émulateur dans la liste déroulante des devices
2. Cliquer sur le bouton **Run** (triangle vert) ou `Ctrl+R`

## Test sur Device Physique

### 1. Activer le mode développeur

Sur le téléphone Android :
1. **Paramètres → À propos du téléphone**
2. Taper 7 fois sur **Numéro de build**
3. Retourner dans **Paramètres → Options pour les développeurs**
4. Activer **Débogage USB**

### 2. Connexion via USB

1. Connecter le téléphone via USB
2. Autoriser le débogage USB sur le téléphone (popup)
3. Vérifier la connexion :

```powershell
$Env:ANDROID_SDK_ROOT\platform-tools\adb devices
```

Vous devriez voir votre appareil listé.

### 3. Installation et lancement

Dans Qt Creator, sélectionner votre device physique et cliquer sur **Run**.

Ou via ligne de commande :

```powershell
adb install -r build-android\android-build\build\outputs\apk\release\coinche-1.0.0-1-release-arm64-v8a.apk
adb shell am start -n com.coinche.game/org.qtproject.qt.android.bindings.QtActivity
```

## Limitations Actuelles

### Architecture Client-Serveur

⚠️ **Important** : L'architecture actuelle utilise un serveur local (`ws://localhost:1234`) qui ne fonctionnera PAS sur Android.

**Pour que le jeu fonctionne sur Android, vous devez :**

1. **Héberger le serveur sur un serveur distant** (voir `REMOTE_SERVER.md`)
2. **Modifier la connexion dans `LoginView.qml`** :

```qml
Component.onCompleted: {
    // Remplacer localhost par l'URL du serveur distant
    networkManager.connectToServer("ws://votre-serveur.com:1234")
}
```

### Base de données SQLite

La base de données locale ne sera pas accessible sur Android. Deux options :
1. **Backend distant** : API REST pour authentification et statistiques
2. **SQLite embarqué** : Copier la base dans le dossier de l'application Android

## Création d'un APK Signé (Release)

Pour publier sur le Play Store, vous devez signer votre APK :

### 1. Générer un keystore

```bash
keytool -genkey -v -keystore coinche-release.keystore -alias coinche -keyalg RSA -keysize 2048 -validity 10000
```

Sauvegarder le fichier `.keystore` en lieu sûr et noter le mot de passe.

### 2. Configurer build.gradle

Éditer `android/build.gradle` :

```gradle
signingConfigs {
    release {
        storeFile file("../coinche-release.keystore")
        storePassword "votre_mot_de_passe_store"
        keyAlias "coinche"
        keyPassword "votre_mot_de_passe_key"
    }
}

buildTypes {
    release {
        signingConfig signingConfigs.release
        // ...
    }
}
```

### 3. Build Release

Dans Qt Creator :
1. **Build → Build Android APK**
2. Sélectionner **Release**
3. L'APK signé sera dans `android-build/build/outputs/apk/release/`

## Dépannage

### Erreur "SDK Build Tools not found"

```powershell
sdkmanager "build-tools;34.0.0"
```

### Erreur "NDK not configured"

Vérifier dans Qt Creator → Preferences → Android que le NDK est bien sélectionné.

### Erreur "JAVA_HOME not set"

```powershell
[System.Environment]::SetEnvironmentVariable("JAVA_HOME", "C:\Program Files\Eclipse Adoptium\jdk-17.x.x", "User")
```
Redémarrer Qt Creator.

### Application crash au lancement

Vérifier les logs avec :

```powershell
adb logcat | Select-String "coinche|Qt"
```

### Taille de l'APK trop grande

L'APK peut être volumineux (50-100 MB) à cause des bibliothèques Qt. Options :
- Utiliser Android App Bundle (AAB) au lieu d'APK
- Activer ProGuard dans `build.gradle`
- Utiliser la compression Qt

## Prochaines Étapes

Après avoir réussi à compiler pour Android :

1. ✅ **Tester sur différents appareils** (différentes tailles d'écran)
2. ✅ **Adapter l'UI pour le tactile** (voir `ANDROID_UI_ADAPTATION.md`)
3. ✅ **Déployer un serveur distant** (voir `REMOTE_SERVER.md`)
4. ⬜ **Optimiser les performances** (profiling Android)
5. ⬜ **Préparer pour le Play Store** (icônes, screenshots, description)

## Resources Utiles

- [Qt for Android Documentation](https://doc.qt.io/qt-6/android.html)
- [Qt Creator Android Setup](https://doc.qt.io/qtcreator/creator-developing-android.html)
- [Android Developers](https://developer.android.com/)
- [Qt Forum - Android](https://forum.qt.io/category/11/android)

## Support

Pour toute question ou problème, consultez :
- La documentation Qt officielle
- Les logs Android via `adb logcat`
- Le forum Qt
