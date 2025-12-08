# Check Android SDK/NDK setup
Write-Host "=== Verification Configuration Android ===" -ForegroundColor Cyan

# Check Android SDK
$sdkPath = "$Env:LOCALAPPDATA\Android\Sdk"
if (Test-Path $sdkPath) {
    Write-Host "Android SDK trouve: $sdkPath" -ForegroundColor Green

    if (Test-Path "$sdkPath\build-tools") {
        $buildTools = Get-ChildItem "$sdkPath\build-tools" | Select-Object -First 1
        Write-Host "Build Tools: $($buildTools.Name)" -ForegroundColor Green
    }

    if (Test-Path "$sdkPath\ndk") {
        $ndk = Get-ChildItem "$sdkPath\ndk" | Select-Object -First 1
        Write-Host "NDK: $($ndk.Name)" -ForegroundColor Green
    }

    if (Test-Path "$sdkPath\platforms") {
        $platforms = Get-ChildItem "$sdkPath\platforms" | Select-Object -Last 1
        Write-Host "Platform: $($platforms.Name)" -ForegroundColor Green
    }
} else {
    Write-Host "Android SDK NON trouve" -ForegroundColor Red
    Write-Host "Installer Android Studio depuis: https://developer.android.com/studio" -ForegroundColor Yellow
}

Write-Host ""

# Check JDK
if ($Env:JAVA_HOME) {
    Write-Host "JAVA_HOME: $Env:JAVA_HOME" -ForegroundColor Green
} else {
    Write-Host "JAVA_HOME non defini" -ForegroundColor Red
    Write-Host "Installer JDK 17: https://adoptium.net/temurin/releases/?version=17" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "Qt Android: C:\Qt\6.9.3\android_arm64_v8a" -ForegroundColor Green
