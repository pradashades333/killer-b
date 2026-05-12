param(
    [string]$Vst3Path = ".\build\KillerBSynth_artefacts\Release\VST3\Killer B Synth.vst3",
    [string]$OutputExe = ".\Killer-B-Synth-Windows-Installer.exe"
)

$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent $PSScriptRoot
$resolvedVst3 = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $Vst3Path))
$resolvedOutput = [System.IO.Path]::GetFullPath((Join-Path $repoRoot $OutputExe))
$nsisScript = Join-Path $repoRoot "Installer\windows-installer.nsi"

if (-not (Test-Path $resolvedVst3)) {
    throw "VST3 bundle not found: $resolvedVst3"
}

if (-not (Test-Path $nsisScript)) {
    throw "NSIS script not found: $nsisScript"
}

$makensis = Get-Command makensis -ErrorAction SilentlyContinue
if (-not $makensis) {
    $candidates = @(
        "C:\Program Files (x86)\NSIS\makensis.exe",
        "C:\Program Files\NSIS\makensis.exe"
    )

    foreach ($candidate in $candidates) {
        if (Test-Path $candidate) {
            $makensis = @{ Source = $candidate }
            break
        }
    }
}

if (-not $makensis) {
    throw "NSIS is not installed. Install it first so makensis is available."
}

$null = New-Item -ItemType Directory -Path (Split-Path -Parent $resolvedOutput) -Force

& $makensis.Source `
    "/DOUTPUT_EXE=$resolvedOutput" `
    "/DVST3_BUNDLE=$resolvedVst3" `
    $nsisScript

if (-not (Test-Path $resolvedOutput)) {
    throw "Installer was not created: $resolvedOutput"
}

Write-Host "Created installer: $resolvedOutput"
