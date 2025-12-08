# Installer OpenSSL pour Qt Android (Optionnel)

OpenSSL est nécessaire pour les connexions sécurisées (HTTPS, WSS) sur Android.

## ⚠️ Est-ce Nécessaire Maintenant ?

**Non** si vous utilisez :
- `ws://` (WebSocket non-sécurisé) ✅
- `http://` ✅

**Oui** si vous utilisez :
- `wss://` (WebSocket sécurisé)
- `https://`

## 📥 Installation via Qt

### Méthode 1 : Qt Maintenance Tool (Plus Simple)

1. Lancer **Qt Maintenance Tool** (`C:\Qt\MaintenanceTool.exe`)
2. "Add or remove components"
3. Développer : **Qt → Qt 6.9.3 → Additional Libraries**
4. Cocher : **OpenSSL for Android**
5. Apply et attendre le téléchargement

### Méthode 2 : Téléchargement Manuel

Si pas disponible dans Maintenance Tool :

1. **Télécharger OpenSSL précompilé pour Android** :
   - https://github.com/KDAB/android_openssl
   - Cliquer sur **Releases** (à droite)
   - Télécharger : `openssl_3.x.x_android.zip`

2. **Extraire** le zip

3. **Copier les bibliothèques** :
   ```
   openssl_3.x.x_android/
   ├─ arm64-v8a/
   │  ├─ libcrypto_3.so
   │  └─ libssl_3.so
   └─ armeabi-v7a/
      ├─ libcrypto_3.so
      └─ libssl_3.so
   ```

4. **Dans Qt Creator** :
   - Edit → Preferences → Devices → Android
   - Section "OpenSSL"
   - Cliquer sur "Download OpenSSL"
   - Ou spécifier le chemin manuellement

## ✅ Vérification

Dans Qt Creator → Preferences → Android :
- La ligne OpenSSL devrait avoir un ✓ vert

## 💡 Recommandation

**Pour l'instant** :
- Ignorez l'avertissement OpenSSL
- Compilez votre premier APK
- Testez le jeu
- Si plus tard vous avez besoin de `wss://` ou `https://`, revenez installer OpenSSL

**L'app fonctionnera parfaitement sans** pour vos tests locaux en `ws://` !

---

Voulez-vous installer OpenSSL maintenant ou continuer sans ?
