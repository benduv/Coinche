# configure_java_home.ps1
# Configure JAVA_HOME pour JDK 17

Write-Host "=== Configuration JAVA_HOME ===" -ForegroundColor Cyan
Write-Host ""

$jdkPath = "C:\Program Files\Eclipse Adoptium\jdk-17.0.17.10-hotspot"

if (Test-Path $jdkPath) {
    Write-Host "JDK trouve: $jdkPath" -ForegroundColor Green
    Write-Host ""
    Write-Host "Configuration de JAVA_HOME..." -ForegroundColor Yellow

    [System.Environment]::SetEnvironmentVariable("JAVA_HOME", $jdkPath, "User")

    Write-Host "JAVA_HOME configure!" -ForegroundColor Green
    Write-Host ""
    Write-Host "Verification:" -ForegroundColor Cyan
    Write-Host "JAVA_HOME = $jdkPath" -ForegroundColor White
    Write-Host ""
    Write-Host "IMPORTANT: Redemarrez Qt Creator pour appliquer les changements!" -ForegroundColor Yellow
    Write-Host ""
    Write-Host "Test (dans un NOUVEAU PowerShell):" -ForegroundColor Cyan
    Write-Host "  java -version" -ForegroundColor White
} else {
    Write-Host "ERREUR: JDK non trouve a $jdkPath" -ForegroundColor Red
}
