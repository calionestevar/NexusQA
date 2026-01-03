#!/usr/bin/env pwsh
<#
.SYNOPSIS
    Extracts HTML template from LCARSTemplate.cpp and writes to LCARSDashboard.html
    
.DESCRIPTION
    This script reads the embedded HTML template from the C++ source file and generates
    the corresponding HTML file for development/packaged builds. This ensures a single
    source of truth while providing convenient editing during development.
    
    Single Source of Truth Pattern:
    - LCARSTemplate.cpp contains the authoritative template
    - This script extracts it to LCARSDashboard.html for dev builds
    - Shipped builds use the embedded template from the compiled C++ file
    
.EXAMPLE
    .\Generate-LCARSTemplate.ps1
    
.NOTES
    Cross-platform compatible (Windows, macOS, Linux with PowerShell 7+)
    Run from project root or specify -ProjectRoot parameter
#>

param(
    [string]$ProjectRoot = (Get-Location),
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

# Use platform-independent path joining
$CppSourcePath = Join-Path $ProjectRoot "Source\Nexus\LCARSBridge\Private\LCARSTemplate.cpp"
$HtmlOutputPath = Join-Path $ProjectRoot "Source\Nexus\LCARSBridge\Private\LCARSTemplate.html"

Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
Write-Host "   HTML Template Generator — Observer Network Dashboard"
Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
Write-Host ""

# Validate source file exists
if (-not (Test-Path $CppSourcePath)) {
    Write-Error "Source file not found: $CppSourcePath"
    exit 1
}

Write-Host "[+] Source: $CppSourcePath"
Write-Host "[+] Output: $HtmlOutputPath"
Write-Host ""

try {
    # Read C++ source
    $CppContent = Get-Content $CppSourcePath -Raw
    
    # Extract HTML from raw string literal
    # Pattern: R"(...)" captures everything between the parentheses
    # Use (?s) flag to allow . to match newlines (singleline/DOTALL mode)
    if ($CppContent -match '(?s)R"\((.+?)\)"') {
        $HtmlContent = $matches[1]
        
        # Write to file with UTF8 encoding (cross-platform compatible)
        Set-Content -Path $HtmlOutputPath -Value $HtmlContent -Encoding UTF8
        
        $fileSize = (Get-Item $HtmlOutputPath).Length / 1KB
        Write-Host "[+] Successfully extracted HTML template"
        Write-Host "[+] File size: $([math]::Round($fileSize, 2)) KB"
        Write-Host "[+] Location: $HtmlOutputPath"
        Write-Host ""
        Write-Host "Generated from: LCARSTemplate.cpp"
        Write-Host "Single source of truth for all LCARS report templates."
        Write-Host "This file is referenced by:"
        Write-Host "  - Generate-LCARSReport.ps1 (development builds)"
        Write-Host "  - generate-lcars-report.sh (development builds)"
        Write-Host "  - LCARSTemplate.cpp (shipped/compiled builds)"
        Write-Host ""
        Write-Host "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━"
    }
    else {
        Write-Error "Could not find HTML template in C++ source. Pattern mismatch."
        exit 1
    }
}
catch {
    Write-Error "Failed to generate HTML template: $_"
    exit 1
}
