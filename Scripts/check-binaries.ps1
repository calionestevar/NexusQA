<#
Simple helper to sanity-check compiled plugin binaries and project receipts.
Run from PowerShell:
  .\Scripts\check-binaries.ps1
#>
param(
    [switch]$Verbose
)

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Definition
$projectRoot = (Resolve-Path (Join-Path $scriptDir "..") ).Path
Write-Host "Project root: $projectRoot"

# Check for project-level Binaries/Win64 receipts
$projectBinariesDir = Join-Path $projectRoot "Binaries\Win64"
$targetFiles = @()
if (Test-Path $projectBinariesDir) {
    $targetFiles = Get-ChildItem -Path $projectBinariesDir -Filter "*.target" -ErrorAction SilentlyContinue
}
if (-not $targetFiles -or $targetFiles.Count -eq 0) {
    Write-Host "Warning: No .target receipts found in $projectBinariesDir" -ForegroundColor Yellow
} else {
    Write-Host "Found .target receipts:" -ForegroundColor Green
    $targetFiles | ForEach-Object { Write-Host " - $($_.Name)" }
}

# Inspect project Plugins
$pluginsDir = Join-Path $projectRoot "Plugins"
if (-not (Test-Path $pluginsDir)) {
    Write-Host "No Plugins folder at $pluginsDir" -ForegroundColor Yellow
} else {
    $pluginDirs = Get-ChildItem -Path $pluginsDir -Directory -ErrorAction SilentlyContinue
    if ($pluginDirs.Count -eq 0) { Write-Host "No plugin directories found." }
    foreach ($p in $pluginDirs) {
        Write-Host "\nPlugin: $($p.Name)" -ForegroundColor Cyan
        $uplugin = Get-ChildItem -Path $p.FullName -Filter "*.uplugin" -Recurse -ErrorAction SilentlyContinue | Select-Object -First 1
        if (-not $uplugin) {
            Write-Host " - No .uplugin file found in plugin folder." -ForegroundColor Yellow
            continue
        }
        Write-Host " - Descriptor: $($uplugin.FullName)"
        try {
            $json = Get-Content $uplugin.FullName -Raw | ConvertFrom-Json
        } catch {
            Write-Host " - Failed to parse .uplugin JSON" -ForegroundColor Red
            continue
        }
        $modules = @()
        if ($json.modules) { $modules = $json.modules | ForEach-Object { $_.name } }

        $pluginBinDir = Join-Path $p.FullName "Binaries\Win64"
        $dlls = @()
        if (Test-Path $pluginBinDir) { $dlls = Get-ChildItem -Path $pluginBinDir -Filter "*.dll" -ErrorAction SilentlyContinue }

        if (-not $dlls -or $dlls.Count -eq 0) {
            Write-Host " - No Win64 binaries found under $pluginBinDir" -ForegroundColor Yellow
        } else {
            Write-Host " - Binaries found:" -ForegroundColor Green
            $dlls | ForEach-Object { Write-Host "    - $($_.Name)" }
        }

        if ($modules.Count -eq 0) {
            Write-Host " - No modules declared in .uplugin" -ForegroundColor Yellow
        } else {
            foreach ($m in $modules) {
                $match = $dlls | Where-Object { $_.Name -match [regex]::Escape($m) }
                if ($match) {
                    Write-Host " - Module ${m}: binary present" -ForegroundColor Green
                } else {
                    # also check project-level Binaries as fallback
                    $fallback = Get-ChildItem -Path (Join-Path $projectBinariesDir "*") -Filter "*$m*.dll" -ErrorAction SilentlyContinue
                    if ($fallback) {
                        Write-Host " - Module ${m}: binary present in project Binaries" -ForegroundColor Green
                    } else {
                        Write-Host " - Module ${m}: MISSING binary" -ForegroundColor Red
                    }
                }
            }
        }
    }
}

Write-Host "\nCheck complete." -ForegroundColor Cyan
