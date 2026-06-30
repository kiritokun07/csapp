# Data Lab - WSL environment setup (run in PowerShell as Administrator)
# Usage: powershell -ExecutionPolicy Bypass -File setup\setup-wsl.ps1

$ErrorActionPreference = "Stop"

function Test-Admin {
    ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
}

if (-not (Test-Admin)) {
    Write-Host "Please run this script as Administrator." -ForegroundColor Red
    exit 1
}

Write-Host "=== Step 1: Enable Windows features ===" -ForegroundColor Cyan
dism.exe /online /enable-feature /featurename:Microsoft-Windows-Subsystem-Linux /all /norestart | Out-Null
dism.exe /online /enable-feature /featurename:VirtualMachinePlatform /all /norestart | Out-Null

Write-Host "=== Step 2: Install WSL (Microsoft Store package) ===" -ForegroundColor Cyan
Write-Host "If this fails with network error, download manually from:"
Write-Host "  https://aka.ms/wslstore"
Write-Host "Then install the downloaded .msixbundle with:"
Write-Host "  Add-AppxPackage -Path <path-to-msixbundle>"
Write-Host ""

winget install Microsoft.WSL --accept-package-agreements --accept-source-agreements

Write-Host "=== Step 3: Install Ubuntu 22.04 ===" -ForegroundColor Cyan
winget install Canonical.Ubuntu.2204 --accept-package-agreements --accept-source-agreements

Write-Host ""
Write-Host "=== Step 4: Reboot if prompted, then open Ubuntu ===" -ForegroundColor Cyan
Write-Host "First launch will ask you to create a Linux username/password."
Write-Host ""
Write-Host "=== Step 5: Inside Ubuntu, run ===" -ForegroundColor Cyan
Write-Host "  cd /mnt/c/github/csapp/no1-datalab"
Write-Host "  bash setup/setup-datalab.sh"
Write-Host ""
