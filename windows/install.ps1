<#
.SYNOPSIS
    Automates deployment of the LibrePods Windows bundle: enables test mode (optional), installs the KMDF driver and copies the Qt app.

.PARAMETER DriverInfPath
    Path to the driver INF file to install with pnputil.

.PARAMETER QtBinaryPath
    Path to the built LibrePods executable that should be deployed.

.PARAMETER InstallDirectory
    Destination directory for the application (default: %ProgramFiles%\LibrePods).

.PARAMETER QtDeployTool
    Optional path to windeployqt.exe to copy Qt runtime dependencies.

.PARAMETER SkipTestMode
    Skip enabling Windows test signing (use when a signed driver is available).
#>
param(
    [string]$DriverInfPath = (Join-Path $PSScriptRoot 'driver\AacpTransportDriver.inf'),
    [string]$QtBinaryPath = (Join-Path $PSScriptRoot 'build\LibrePods.exe'),
    [string]$InstallDirectory = (Join-Path ${env:ProgramFiles} 'LibrePods'),
    [string]$QtDeployTool = 'windeployqt.exe',
    [switch]$SkipTestMode
)

function Assert-Admin {
    $currentIdentity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($currentIdentity)
    if (-not $principal.IsInRole([Security.Principal.WindowsBuiltinRole]::Administrator)) {
        throw 'Bitte führe das Skript als Administrator aus.'
    }
}

function Enable-TestSigning {
    param([switch]$Force)

    if ($SkipTestMode -and -not $Force) { return }

    Write-Host 'Aktiviere Windows Testmodus (bcdedit /set testsigning on)...'
    bcdedit /set testsigning on | Out-Null
    Write-Host 'Bitte starte Windows neu, falls der Testmodus gerade aktiviert wurde.'
}

function Install-KmdfDriver {
    param([string]$InfPath)

    if (-not (Test-Path $InfPath)) {
        throw "INF-Datei nicht gefunden: $InfPath"
    }

    Write-Host "Installiere KMDF-Treiber aus $InfPath ..."
    pnputil /add-driver $InfPath /install /subdirs
}

function Deploy-QtApp {
    param(
        [string]$BinaryPath,
        [string]$TargetDirectory,
        [string]$DeployTool
    )

    if (-not (Test-Path $BinaryPath)) {
        throw "LibrePods-Binärdatei nicht gefunden: $BinaryPath"
    }

    New-Item -ItemType Directory -Force -Path $TargetDirectory | Out-Null
    Copy-Item -Force -Path $BinaryPath -Destination (Join-Path $TargetDirectory 'LibrePods.exe')

    if (Get-Command $DeployTool -ErrorAction SilentlyContinue) {
        Write-Host 'Kopiere Qt-Abhängigkeiten mit windeployqt...'
        & $DeployTool (Join-Path $TargetDirectory 'LibrePods.exe') --dir $TargetDirectory --release
    } else {
        Write-Warning "windeployqt nicht gefunden ($DeployTool). Bitte kopiere Qt DLLs manuell falls nötig."
    }
}

try {
    Assert-Admin
    Enable-TestSigning
    Install-KmdfDriver -InfPath (Resolve-Path $DriverInfPath)
    Deploy-QtApp -BinaryPath (Resolve-Path $QtBinaryPath) -TargetDirectory $InstallDirectory -DeployTool $QtDeployTool

    Write-Host "Fertig! Starte LibrePods aus $InstallDirectory."
}
catch {
    Write-Error $_
    exit 1
}
