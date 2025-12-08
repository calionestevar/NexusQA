<#
Engage.ps1 - Canonical Windows test harness for Nexus
Keeps the Star Trek flavor while being explicit about what it does.
Usage: run this from the project root. It will attempt to locate your UnrealEditor-Cmd.exe
via the UE_ENGINE_PATH env var or common install locations.
#>

# Accept an optional EnginePath parameter to override auto-detection.
param(
    [string]$EnginePath
)

# Startup checks: if ExecutionPolicy prevents running scripts, print guidance
# and exit early. This helps less-experienced users who encounter the
# "running scripts is disabled on this system" error.
try {
    $policy = Get-ExecutionPolicy -ErrorAction Stop
} catch {
    $policy = 'Undefined'
}
if ($policy -eq 'Restricted') {
    Write-Host "ExecutionPolicy is 'Restricted' - scripts are blocked on this system." -ForegroundColor Yellow
    Write-Host "To run once without changing policy, use the batch wrapper (recommended):" -ForegroundColor Yellow
    Write-Host "    .\Scripts\Engage.bat" -ForegroundColor Yellow
    Write-Host "Or run PowerShell with a temporary bypass:" -ForegroundColor Yellow
    Write-Host "    powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\Engage.ps1" -ForegroundColor Yellow
    exit 2
}

Write-Host "`n═══════════════════════════════════════════════════"
Write-Host "           NEXUS ENGAGED - STARFLEET PROTOCOL"
Write-Host "           Initiating automated test harness"
Write-Host "═══════════════════════════════════════════════════`n"

# Auto-detect engine executable
if ($EnginePath) {
    $EngineExe = Join-Path $EnginePath "Binaries\Win64\UnrealEditor-Cmd.exe"
} else {
    $EngineExe = "$Env:UE_ENGINE_PATH\Binaries\Win64\UnrealEditor-Cmd.exe"
    if (-not (Test-Path $EngineExe)) {
        # Try common install path (adjust if your engine version differs)
        $EngineExe = "C:\Program Files\Epic Games\UE_5.0\Engine\Binaries\Win64\UnrealEditor-Cmd.exe"
    }
}

if (-not (Test-Path $EngineExe)) {
    Write-Host "UnrealEditor-Cmd.exe not found. Generating sample artifacts for demo..." -ForegroundColor Yellow
    
    # Generate sample artifacts so recruiters can preview reports without building UE
    $ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
    $RepoRoot = Split-Path -Parent $ScriptDir
    $reportDir = Join-Path $RepoRoot "Saved\NexusReports"
    if (-not (Test-Path $reportDir)) { New-Item -ItemType Directory -Path $reportDir -Force | Out-Null }

    # Sample JSON
    $json = @"
{
  "generated": "$(Get-Date -Format o)",
  "tests": [
    { "name": "Smoke.Test1", "status": "PASSED", "duration": 0.123 },
    { "name": "Smoke.Test2", "status": "FAILED", "duration": 0.456, "artifacts": ["test_Smoke_Test2.log"] }
  ]
}
"@
    $jsonPath = Join-Path $reportDir "LCARSReport.json"
    Set-Content -Path $jsonPath -Value $json -Encoding UTF8

    # Sample HTML
    $html = @"
<!doctype html>
<html><head><title>LCARS Report</title><style>body{font-family:monospace;background:#000;color:#00ff00;padding:20px}</style></head>
<body><h1>Sample LCARS Report</h1><p>Generated: $(Get-Date)</p><p>This is a demo report. Run Nexus with your UE engine for real results.</p></body></html>
"@
    $htmlPath = Join-Path $reportDir "LCARS_Report_sample.html"
    Set-Content -Path $htmlPath -Value $html -Encoding UTF8

    # Sample JUnit XML
    $xml = "<?xml version='1.0' encoding='UTF-8'?><testsuites><testsuite name='Nexus' tests='2' failures='1'><testcase classname='NexusTests' name='Smoke.Test1' time='0.123'/><testcase classname='NexusTests' name='Smoke.Test2' time='0.456'><failure message='failed'>Assertion</failure></testcase></testsuite></testsuites>"
    $xmlPath = Join-Path $reportDir "nexus-results.xml"
    Set-Content -Path $xmlPath -Value $xml -Encoding UTF8

    Write-Host "Sample artifacts written to: $reportDir" -ForegroundColor Green
    exit 0
}

$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$RepoRoot = Split-Path -Parent $ScriptDir
$ProjectFile = Join-Path $RepoRoot "NexusDemo.uproject"
if (-not (Test-Path $ProjectFile)) {
    Write-Host "Project file not found at $ProjectFile" -ForegroundColor Red
    exit 3
}

Write-Host "Project locked: $ProjectFile`n"

# Remove any existing sentinel
$AbortFile = Join-Path $RepoRoot "Saved\NexusAbort.flag"
if (Test-Path $AbortFile) { Remove-Item $AbortFile -Force }

# As with the original Engage.bat, we allow a -legacy flag to run legacy mode
$ArgsList = $args -join ' '
$Mode = 'NEXUS'
if ($args -contains '-legacy' -or $args -contains '--legacy') { $Mode = 'LEGACY' }

if ($Mode -eq 'LEGACY') {
    Write-Host "[LEGACY MODE] Asgard legacy path - invoking classic commandlet..."
    & $EngineExe "`"$ProjectFile`"" -run=Asgard -NullRHI -nosound -log -unattended
    exit $LASTEXITCODE
}

Write-Host "[NEXUS MODE] Fleet deployment: Nexus test realms launching..."

$ExecCmds = "Nexus.Execute $ArgsList; Quit"
& $EngineExe "`"$ProjectFile`"" -nullrhi -nosound -unattended -log -ExecCmds=$ExecCmds

$Result = $LASTEXITCODE

if ($Result -ne 0) {
    Write-Host "`nPALANTIR ACTIVATED - RECORDING FAILURE FOR ANALYSIS" -ForegroundColor Yellow
    & $EngineExe "`"$ProjectFile`"" -game -run=PalantirCapture -NullRHI -log
}

Write-Host "`n═══════════════════════════════════════════════════"
if ($Result -eq 0) {
    Write-Host "           VICTORY! The fleet reports all clear." -ForegroundColor Green
} else {
    Write-Host "           ALERT - Critical failures detected. PALANTIR engaged." -ForegroundColor Red
}
Write-Host "═══════════════════════════════════════════════════`n"

exit $Result
