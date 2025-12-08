<#
install-imgui.ps1

Download and install an ImGui plugin into the project-local `Plugins/ImGui` folder.

Usage:
  .\Scripts\install-imgui.ps1 -Url <download-zip-or-github-release-zip>

If no URL is supplied, the script will prompt you to paste a zip URL (GitHub release or raw zip).
The script will extract the archive into `Plugins/ImGui` (overwriting the stub) and then
generate project files using your configured `UE_ENGINE_PATH` or the `-EnginePath` argument.

This helper does not auto-build the engine/project — run Build.bat or open the solution in Visual Studio.
#>

param(
    [string]$Url,
    [string]$EnginePath
)

if (-not $Url) {
    Write-Host "Enter a download URL to an ImGui plugin zip (GitHub release, etc.):"
    $Url = Read-Host
}

if (-not $Url) { Write-Error "No URL provided. Aborting."; exit 1 }

$RepoRoot = Split-Path -Parent $MyInvocation.MyCommand.Path | Split-Path -Parent
$PluginsDir = Join-Path $RepoRoot "Plugins"
$TargetDir = Join-Path $PluginsDir "ImGui"

Write-Host "Downloading ImGui plugin from: $Url"
$Tmp = [System.IO.Path]::GetTempFileName() + ".zip"
Invoke-WebRequest -Uri $Url -OutFile $Tmp -UseBasicParsing

if (Test-Path $TargetDir) {
    Write-Host "Removing existing $TargetDir"
    Remove-Item -Recurse -Force $TargetDir
}

Write-Host "Extracting plugin to $PluginsDir"
Expand-Archive -LiteralPath $Tmp -DestinationPath $PluginsDir

# Try to find a top-level folder that contains an ImGui.uplugin
$found = Get-ChildItem -Path $PluginsDir -Directory | Where-Object {
    Test-Path (Join-Path $_.FullName "ImGui.uplugin")
}

if ($found.Count -eq 0) {
    # Maybe the zip extracted a folder that itself contains the plugin folder
    $nested = Get-ChildItem -Path $PluginsDir -Directory -Recurse -Depth 2 | Where-Object {
        Test-Path (Join-Path $_.FullName "ImGui.uplugin")
    }
    if ($nested.Count -gt 0) { $found = $nested }
}

if ($found.Count -gt 0) {
    $pluginRoot = $found[0].FullName
    if ($pluginRoot -ne $TargetDir) {
        Write-Host "Moving plugin folder to $TargetDir"
        Move-Item -Path $pluginRoot -Destination $TargetDir
    }
    Write-Host "ImGui plugin installed to $TargetDir"
} else {
    Write-Warning "Could not find ImGui.uplugin in the extracted archive. Please move the plugin folder into Plugins/ImGui manually."
}

Remove-Item $Tmp -Force

if ($EnginePath) {
    $gen = Join-Path $EnginePath "Build\BatchFiles\GenerateProjectFiles.bat"
    if (Test-Path $gen) {
        Write-Host "Generating project files using engine at $EnginePath"
        & $gen -projectfiles -project="$(Join-Path $RepoRoot 'NexusDemo.uproject')" -game -engine
    } else {
        Write-Warning "GenerateProjectFiles.bat not found under $EnginePath. Please run it manually."
    }
} else {
    Write-Host "Plugin installed. Run GenerateProjectFiles.bat with your engine to pick it up, e.g."
    Write-Host "& 'E:\\Epic Games\\UE_5.6\\Engine\\Build\\BatchFiles\\GenerateProjectFiles.bat' -projectfiles -project=\"$RepoRoot\NexusDemo.uproject\" -game -engine"
}

Write-Host "Done."
