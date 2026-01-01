<#
.SYNOPSIS
    Extracts HTML template from ObserverNetworkTemplate.cpp and writes to ObserverNetworkDashboard.html
    
.DESCRIPTION
    This script reads the embedded HTML template from the C++ source file and generates
    the corresponding HTML file for development/packaged builds. This ensures a single
    source of truth while providing convenient editing during development.
    
    Single Source of Truth Pattern:
    - ObserverNetworkTemplate.cpp contains the authoritative template
    - This script extracts it to ObserverNetworkDashboard.html for dev builds
    - Shipped builds use the embedded template from the compiled C++ file
    
.EXAMPLE
    .\GenerateHTMLTemplate.ps1
    
.NOTES
    Run from project root or specify -ProjectRoot parameter
#>

param(
    [string]$ProjectRoot = (Get-Location),
    [switch]$Verbose
)

$ErrorActionPreference = "Stop"

# Paths
$CppSourcePath = Join-Path $ProjectRoot "Source\FringeNetwork\Private\ObserverNetworkTemplate.cpp"
$HtmlOutputPath = Join-Path $ProjectRoot "Source\FringeNetwork\Private\ObserverNetworkDashboard.html"

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
    if ($CppContent -match 'R"\((.+)\)"') {
        $HtmlContent = $matches[1]
        
        # Write to file
        Set-Content -Path $HtmlOutputPath -Value $HtmlContent -Encoding UTF8
        
        $fileSize = (Get-Item $HtmlOutputPath).Length / 1KB
        Write-Host "[✓] Successfully extracted HTML template"
        Write-Host "[✓] File size: $([math]::Round($fileSize, 2)) KB"
        Write-Host "[✓] Location: $HtmlOutputPath"
        Write-Host ""
        Write-Host "Generated from: ObserverNetworkTemplate.cpp"
        Write-Host "This is the development/packaged builds version."
        Write-Host "Shipped builds use the embedded C++ template."
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
