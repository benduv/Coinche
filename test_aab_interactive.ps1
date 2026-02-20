# Script PowerShell pour tester l'AAB sur un telephone Android (avec mot de passe interactif)

$aabFile = "build\Android_Combined_Release\android-build-coinche\build\outputs\bundle\release\coinche-release.aab"
$bundletoolJar = "bundletool.jar"
$apksOutput = "coinche-test.apks"

Write-Host "=== Test de l'AAB sur telephone Android ===" -ForegroundColor Cyan

# Verifier que l'AAB existe
if (-not (Test-Path $aabFile)) {
    Write-Host "ERREUR: L'AAB n'existe pas a $aabFile" -ForegroundColor Red
    exit 1
}

# Verifier que bundletool existe
if (-not (Test-Path $bundletoolJar)) {
    Write-Host "ERREUR: bundletool.jar introuvable" -ForegroundColor Red
    Write-Host "Executez d'abord le script de telechargement" -ForegroundColor Yellow
    exit 1
}

# Trouver ADB
$adbPath = "adb"
$adbSdkPath = "$env:LOCALAPPDATA\Android\Sdk\platform-tools\adb.exe"
if (Test-Path $adbSdkPath) {
    $adbPath = $adbSdkPath
}

Write-Host ""
Write-Host "=== Verification du telephone connecte ===" -ForegroundColor Cyan
$devices = & $adbPath devices
Write-Host $devices

if ($devices -match "device$") {
    Write-Host ""
    Write-Host "Telephone detecte!" -ForegroundColor Green

    # Supprimer l'ancien fichier APKS s'il existe
    if (Test-Path $apksOutput) {
        Remove-Item $apksOutput -Force
    }

    # Demander si on utilise la signature
    Write-Host ""
    $useSignature = Read-Host "Voulez-vous signer les APK? (o/n) [n]"

    Write-Host ""
    Write-Host "=== Creation des APK depuis l'AAB ===" -ForegroundColor Cyan

    if ($useSignature -eq "o") {
        $keystorePath = "coinche-release-key.keystore"
        if (Test-Path $keystorePath) {
            Write-Host "Utilisation de la cle de signature: $keystorePath" -ForegroundColor Yellow
            Write-Host ""

            # Demander le mot de passe
            $keystorePassword = Read-Host "Mot de passe du keystore" -AsSecureString
            $keystorePasswordPlain = [Runtime.InteropServices.Marshal]::PtrToStringAuto([Runtime.InteropServices.Marshal]::SecureStringToBSTR($keystorePassword))

            $keyPassword = Read-Host "Mot de passe de la cle (appuyez sur Entree si identique)" -AsSecureString
            $keyPasswordPlain = [Runtime.InteropServices.Marshal]::PtrToStringAuto([Runtime.InteropServices.Marshal]::SecureStringToBSTR($keyPassword))

            if ([string]::IsNullOrEmpty($keyPasswordPlain)) {
                $keyPasswordPlain = $keystorePasswordPlain
            }

            Write-Host ""
            Write-Host "Creation des APK signes..." -ForegroundColor Yellow
            java -jar $bundletoolJar build-apks --bundle=$aabFile --output=$apksOutput --mode=default --connected-device --adb=$adbPath --ks=$keystorePath --ks-key-alias=coinche --ks-pass=pass:$keystorePasswordPlain --key-pass=pass:$keyPasswordPlain
        }
        else {
            Write-Host "ERREUR: Keystore introuvable a $keystorePath" -ForegroundColor Red
            exit 1
        }
    }
    else {
        Write-Host "Creation d'APK non signes pour test..." -ForegroundColor Yellow
        java -jar $bundletoolJar build-apks --bundle=$aabFile --output=$apksOutput --mode=default --connected-device --adb=$adbPath
    }

    if ($LASTEXITCODE -eq 0) {
        Write-Host ""
        Write-Host "APK crees avec succes!" -ForegroundColor Green

        Write-Host ""
        Write-Host "=== Installation sur le telephone ===" -ForegroundColor Cyan
        java -jar $bundletoolJar install-apks --apks=$apksOutput --adb=$adbPath

        if ($LASTEXITCODE -eq 0) {
            Write-Host ""
            Write-Host "Application installee avec succes!" -ForegroundColor Green
            Write-Host ""
            Write-Host "Vous pouvez maintenant tester l'application sur votre telephone" -ForegroundColor Cyan

            # Obtenir l'architecture du telephone
            Write-Host ""
            Write-Host "Architecture du telephone:" -ForegroundColor Cyan
            & $adbPath shell getprop ro.product.cpu.abi
        }
        else {
            Write-Host ""
            Write-Host "ERREUR lors de l'installation" -ForegroundColor Red
        }
    }
    else {
        Write-Host ""
        Write-Host "ERREUR lors de la creation des APK" -ForegroundColor Red
    }
}
else {
    Write-Host ""
    Write-Host "Aucun telephone detecte!" -ForegroundColor Red
}
