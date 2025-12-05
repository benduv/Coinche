# Branche Android - Jeu de Coinche

Cette branche contient la configuration pour compiler le jeu de Coinche sur Android.

## État Actuel

✅ **Configuration terminée** :
- AndroidManifest.xml avec permissions réseau
- CMakeLists.txt adapté pour Android
- build.gradle configuré
- Fichiers de ressources Android (strings.xml, styles.xml)
- Configuration Qt Deploy (qtdeploy.json)
- Documentation complète de build

⚠️ **Travail restant** :
- Héberger le serveur sur un serveur distant (actuellement localhost)
- Adapter l'UI pour les écrans tactiles
- Tester sur différents appareils Android
- Optimiser les performances mobiles
- Créer les assets (icônes, splash screen)

## Fichiers Ajoutés/Modifiés

### Nouveaux Fichiers

```
android/
├── AndroidManifest.xml          # Configuration de l'application Android
├── build.gradle                 # Configuration de build Gradle
├── qtdeploy.json               # Configuration Qt pour déploiement
└── res/
    └── values/
        ├── strings.xml          # Chaînes de l'application
        └── styles.xml           # Thème Android
```

### Fichiers Modifiés

- **CMakeLists.txt** :
  - Ajout de la configuration Android
  - Exclusion du serveur et des tests sur Android
  - Support des architectures ARM64/ARMv7

## Quick Start

### 1. Prérequis

Installer :
- Qt 6.5+ avec support Android
- Android Studio ou Android SDK
- Android NDK 26.1.10909125
- JDK 17

### 2. Configuration Qt Creator

1. Ouvrir Qt Creator
2. **Edit → Preferences → Devices → Android**
3. Configurer les chemins SDK/NDK/JDK
4. Vérifier que tout est ✓ en vert

### 3. Build

1. Ouvrir le projet (`CMakeLists.txt`)
2. Sélectionner le kit Android ARM64
3. **Build → Build Android APK**
4. L'APK sera dans `build/android-build/build/outputs/apk/`

📖 **Documentation complète** : Voir [ANDROID_BUILD.md](ANDROID_BUILD.md)

## Architecture Réseau

### Problème Actuel

Le jeu utilise actuellement un serveur local Windows :

```qml
// LoginView.qml ligne 42
networkManager.connectToServer("ws://localhost:1234")
```

**Ceci ne fonctionne PAS sur Android** car :
- Pas de serveur local sur mobile
- `localhost` pointe vers le device lui-même

### Solutions

#### Option 1 : Serveur Cloud (Recommandé)

Héberger le serveur sur un VPS/cloud :

```qml
// À modifier dans LoginView.qml
networkManager.connectToServer("ws://votre-serveur.com:1234")
```

Plateformes suggérées :
- DigitalOcean (5$/mois)
- AWS EC2 (tier gratuit 1 an)
- Heroku
- Railway.app

#### Option 2 : Firebase/Supabase

Remplacer l'architecture client-serveur par un backend-as-a-service.

#### Option 3 : P2P (Avancé)

Utiliser WebRTC pour connexion peer-to-peer sans serveur centralisé.

## Adaptations UI Nécessaires

### Responsive Design

Le jeu utilise déjà des ratios responsive, mais nécessite des ajustements :

1. **Tailles de boutons** : Augmenter pour tactile (min 48dp)
2. **Hover effects** : Remplacer par pressed/tap states
3. **Orientation** : Tester portrait et landscape
4. **Gestures** : Implémenter swipe pour actions

### Exemples de Modifications

```qml
// Avant (Desktop)
Button {
    Layout.preferredWidth: 250
    hovered: true  // Ne fonctionne pas sur tactile
}

// Après (Mobile)
Button {
    Layout.preferredWidth: 300 * root.minRatio
    Layout.minimumHeight: 48  // Taille minimum tactile
    down: true  // État pressed au lieu de hovered
}
```

## Base de Données

### Situation Actuelle

Le jeu utilise SQLite local (`coinche.db`) pour :
- Comptes utilisateurs
- Statistiques
- Authentification

### Sur Android

Deux approches :

1. **SQLite embarqué** : Copier la base dans le stockage de l'app
   ```cpp
   // À implémenter dans DatabaseManager.cpp
   QString dbPath = QStandardPaths::writableLocation(
       QStandardPaths::AppDataLocation) + "/coinche.db";
   ```

2. **Backend API** : Migrer vers une base distante
   - API REST pour auth/stats
   - Firebase Firestore
   - Supabase (PostgreSQL)

## Testing

### Sur Émulateur

```powershell
# Lancer l'émulateur
$Env:ANDROID_SDK_ROOT\emulator\emulator -avd Pixel_5_API_34

# Installer l'APK
adb install -r coinche.apk

# Voir les logs
adb logcat | Select-String "Qt|coinche"
```

### Sur Device Physique

1. Activer **Options développeur** sur le téléphone
2. Activer **Débogage USB**
3. Connecter via USB
4. Dans Qt Creator, sélectionner le device et Run

## Checklist de Publication

Avant de publier sur Google Play Store :

- [ ] Créer un serveur distant fonctionnel
- [ ] Adapter toute l'UI pour tactile
- [ ] Tester sur 3+ appareils différents
- [ ] Créer icône haute résolution (512x512)
- [ ] Screenshots pour différentes tailles
- [ ] Écrire description et politique de confidentialité
- [ ] Générer keystore de signature
- [ ] Build APK signé release
- [ ] Tester l'APK release (pas debug)
- [ ] Vérifier taille APK (<150MB)
- [ ] Payer frais développeur Google ($25)

## Structure du Projet Android

```
coinche/
├── android/                     # Configuration Android
│   ├── AndroidManifest.xml
│   ├── build.gradle
│   ├── qtdeploy.json
│   └── res/
│       ├── drawable/           # Icônes (à créer)
│       │   ├── icon.png
│       │   └── splash.png
│       └── values/
│           ├── strings.xml
│           └── styles.xml
├── qml/                        # Interface QML (à adapter)
├── CMakeLists.txt              # Build config (modifié)
├── ANDROID_BUILD.md            # Documentation détaillée
└── ANDROID_README.md           # Ce fichier
```

## Prochaines Actions Recommandées

### Court Terme (1-2 semaines)

1. **Configurer un serveur de test** :
   - Louer un VPS (DigitalOcean/AWS)
   - Installer le serveur Coinche
   - Modifier `LoginView.qml` pour pointer vers ce serveur
   - Tester la connexion depuis Android

2. **Adapter l'UI principale** :
   - Tester sur émulateur
   - Identifier les éléments trop petits/grands
   - Ajuster les tailles des boutons
   - Remplacer hover par pressed states

### Moyen Terme (2-4 semaines)

3. **Tests intensifs** :
   - Tester sur différents appareils
   - Différentes résolutions d'écran
   - Portrait et landscape
   - Performances (FPS, batterie)

4. **Optimisations** :
   - Profiling avec Android Studio
   - Réduction taille APK
   - Optimisation consommation batterie

### Long Terme (1-2 mois)

5. **Préparation Play Store** :
   - Assets graphiques (icônes, screenshots)
   - Textes marketing
   - Politique de confidentialité
   - Conditions d'utilisation
   - Page Play Store complète

6. **Soumission** :
   - Build release signé
   - Upload sur Play Console
   - Remplir toutes les informations
   - Soumettre pour review

## Commandes Utiles

```powershell
# Vérifier les devices connectés
adb devices

# Installer l'APK
adb install -r coinche.apk

# Désinstaller
adb uninstall com.coinche.game

# Lancer l'app
adb shell am start -n com.coinche.game/org.qtproject.qt.android.bindings.QtActivity

# Voir les logs
adb logcat -c  # Clear logs
adb logcat | Select-String "coinche"

# Screenshot
adb shell screencap -p /sdcard/screen.png
adb pull /sdcard/screen.png

# Informations device
adb shell getprop ro.build.version.sdk  # API level
adb shell wm size  # Résolution écran
```

## Support et Ressources

- **Documentation** : [ANDROID_BUILD.md](ANDROID_BUILD.md)
- **Qt Android Docs** : https://doc.qt.io/qt-6/android.html
- **Android Developer** : https://developer.android.com/
- **Qt Forum** : https://forum.qt.io/category/11/android

## Contributeurs

Cette configuration Android a été créée pour permettre le portage du jeu de Coinche sur mobile.

---

**Note** : Cette branche est un work-in-progress. Le jeu nécessite encore des adaptations pour être pleinement fonctionnel sur Android.
