#!/usr/bin/env pwsh
# Generate LCARS Enhanced Report
# Generates a Starfleet-themed LCARS dashboard with metrics, test grouping, and collapsible sections
# Cross-platform compatible (Windows, macOS, Linux with PowerShell 7+)
#
# Template Architecture: Single Source of Truth
# - Template loaded from LCARSTemplate.html (extracted from LCARSTemplate.cpp)
# - To update template, edit LCARSTemplate.cpp and run Generate-LCARSTemplate.ps1

param(
    [string]$ReportPath = "Saved\NexusReports\LCARS_Report_$(Get-Date -Format 'yyyyMMdd_HHmmss').html",
    [switch]$OpenInBrowser
)

$ReportDir = Split-Path $ReportPath -Parent

# Create directory if it doesn't exist
if (-not (Test-Path $ReportDir)) {
    New-Item -ItemType Directory -Path $ReportDir -Force | Out-Null
}

Write-Host "Generating enhanced LCARS report..." -ForegroundColor Cyan

# Load the template from the extracted HTML file
# Path: Source/Nexus/LCARSBridge/Private/LCARSTemplate.html
$ScriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$TemplateDir = Join-Path (Split-Path -Parent $ScriptDir) "Source\Nexus\LCARSBridge\Private"
$TemplatePath = Join-Path $TemplateDir "LCARSTemplate.html"

if (-not (Test-Path $TemplatePath)) {
    Write-Error "Template file not found: $TemplatePath"
    Write-Host ""
    Write-Host "To generate the template, run:" -ForegroundColor Yellow
    Write-Host "   .\Scripts\Generate-LCARSTemplate.ps1" -ForegroundColor White
    exit 1
}

$templateHtml = Get-Content $TemplatePath -Raw

# Define test data for sample report
$sampleTests = @(
    @{ Name = "Test_NetworkRequest_Basic"; Passed = $true; Tag = "Networking"; Duration = 0.245 },
    @{ Name = "Test_NetworkRequest_Timeout"; Passed = $true; Tag = "Networking"; Duration = 0.312 },
    @{ Name = "Test_NetworkRequest_Retry"; Passed = $false; Tag = "Networking"; Duration = 2.145 },
    @{ Name = "Test_PerformanceBaseline"; Passed = $true; Tag = "Performance"; Duration = 0.089 },
    @{ Name = "Test_PerformanceStress"; Passed = $false; Tag = "Performance"; Duration = 5.234 },
    @{ Name = "Test_GameplayMovement"; Passed = $true; Tag = "Gameplay"; Duration = 0.123 },
    @{ Name = "Test_GameplayCollision"; Passed = $true; Tag = "Gameplay"; Duration = 0.156 },
    @{ Name = "Test_GameplayAI"; Passed = $true; Tag = "Gameplay"; Duration = 0.198 },
    @{ Name = "Test_ComplianceGDPR"; Passed = $true; Tag = "Compliance"; Duration = 0.045 },
    @{ Name = "Test_ComplianceCOPPA"; Passed = $true; Tag = "Compliance"; Duration = 0.052 },
    @{ Name = "Test_IntegrationDatabase"; Passed = $true; Tag = "Integration"; Duration = 0.334 },
    @{ Name = "Test_IntegrationCache"; Passed = $true; Tag = "Integration"; Duration = 0.267 },
    @{ Name = "Test_StressLoad"; Passed = $false; Tag = "Stress"; Duration = 8.912 },
    @{ Name = "Test_StressMemory"; Passed = $true; Tag = "Stress"; Duration = 3.456 },
    @{ Name = "Test_EditorPlugin"; Passed = $true; Tag = "Editor"; Duration = 0.523 },
    @{ Name = "Test_RenderingMesh"; Passed = $true; Tag = "Rendering"; Duration = 0.678 },
    @{ Name = "Test_RenderingShader"; Passed = $true; Tag = "Rendering"; Duration = 0.234 }
)

# Calculate metrics
$totalTests = $sampleTests.Count
$passedTests = ($sampleTests | Where-Object { $_.Passed }).Count
$failedTests = ($sampleTests | Where-Object { -not $_.Passed }).Count
$integrityPercent = [math]::Round(($passedTests / $totalTests) * 100, 1)
$integrityClass = if ($integrityPercent -lt 70) { "critical" } elseif ($integrityPercent -lt 85) { "warning" } else { "" }
$avgDuration = [math]::Round(($sampleTests.Duration | Measure-Object -Average).Average * 1000, 0)
$perfStatus = if ($avgDuration -lt 100) { "Excellent" } elseif ($avgDuration -lt 200) { "Good" } else { "Needs review" }
$regressionCount = ($sampleTests | Where-Object { $_.Duration -gt 1000 }).Count
$regressionStatus = if ($regressionCount -eq 0) { "All clear" } else { "Investigate" }
$criticalTests = 5

# Group tests by tag
$tagGroups = @{
    "Networking" = @()
    "Performance" = @()
    "Gameplay" = @()
    "Compliance" = @()
    "Integration" = @()
    "Stress" = @()
    "Editor" = @()
    "Rendering" = @()
}

foreach ($test in $sampleTests) {
    if ($tagGroups.ContainsKey($test.Tag)) {
        $tagGroups[$test.Tag] += $test
    }
}

# Build tag distribution cards
$tagCards = ""
foreach ($tag in $tagGroups.Keys) {
    $count = $tagGroups[$tag].Count
    $tagCards += @"
                <div class="tag-card">
                    <div class="count">$count</div>
                    <div class="label">$tag</div>
                </div>
"@
}

# Build grouped test sections (collapsible)
$groupedSections = ""
foreach ($tag in $tagGroups.Keys) {
    $tests = $tagGroups[$tag]
    if ($tests.Count -gt 0) {
        $tagPassCount = ($tests | Where-Object { $_.Passed }).Count
        $passPercent = [math]::Round(($tagPassCount / $tests.Count) * 100, 1)
        $hasFailures = ($tests | Where-Object { -not $_.Passed }).Count -gt 0
        $openClass = if ($hasFailures) { " open" } else { "" }
        
        $groupedSections += @"
        <div class="tag-section">
            <div class="tag-section-header" onclick="toggleSection(this)">
                <span>$tag Tests</span>
                <span class="toggle-icon$openClass">&#x25BC;</span>
            </div>
            <div class="tag-section-stats">$($tests.Count) tests - $passPercent% passed</div>
            <div class="tag-section-content$openClass">
                <table class="tag-test-table">
"@
        
        foreach ($test in $tests) {
            $statusClass = if ($test.Passed) { "test-passed" } else { "test-failed" }
            $statusText = if ($test.Passed) { "PASSED" } else { "FAILED" }
            $groupedSections += @"
                    <tr>
                        <td class="$statusClass">$($test.Name) - $statusText</td>
                    </tr>
"@
        }
        
        $groupedSections += @"
                </table>
            </div>
        </div>
"@
    }
}

# Build flat test table rows
$tableRows = ""
foreach ($test in $sampleTests) {
    $statusClass = if ($test.Passed) { "test-passed" } else { "test-failed" }
    $statusText = if ($test.Passed) { "PASSED" } else { "FAILED" }
    $tableRows += "                <tr><td class='test-name'>$($test.Name)</td><td class='$statusClass'>$statusText</td></tr>`n"
}

# For now, create a simple standalone HTML version using PowerShell
# (Later this will be generated via UE5 commandlet)

# Replace placeholders in template with generated data
$html = $templateHtml `
    -replace "{STARDATE}", (Get-Date -Format 'yyyy-MM-dd HH:mm:ss') `
    -replace "{INTEGRITY_PERCENT}", $integrityPercent `
    -replace "{INTEGRITY_CLASS}", $integrityClass `
    -replace "{PASSED_TESTS}", $passedTests `
    -replace "{FAILED_TESTS}", $failedTests `
    -replace "{TOTAL_TESTS}", $totalTests `
    -replace "{CRITICAL_TESTS}", $criticalTests `
    -replace "{AVG_DURATION}", $avgDuration `
    -replace "{PERF_STATUS}", $perfStatus `
    -replace "{REGRESSION_COUNT}", $regressionCount `
    -replace "{REGRESSION_STATUS}", $regressionStatus `
    -replace "{TAG_DISTRIBUTION_CARDS}", $tagCards `
    -replace "{GROUPED_TEST_SECTIONS}", $groupedSections `
    -replace "{ALL_TESTS_TABLE_ROWS}", $tableRows

# Debug: Show what we're injecting (optional)
Write-Host "Template substitution complete" -ForegroundColor Green

$html | Out-File -FilePath $ReportPath -Encoding UTF8

Write-Host "[SUCCESS] Report generated successfully!" -ForegroundColor Green
Write-Host "Location: $ReportPath" -ForegroundColor Yellow
Write-Host ""

if ($OpenInBrowser) {
    Start-Process $ReportPath
    Write-Host "Opening in browser..." -ForegroundColor Cyan
} else {
    Write-Host "To open in browser, run:" -ForegroundColor Cyan
    Write-Host "   Start-Process '$ReportPath'" -ForegroundColor White
}

Write-Host ""
Write-Host "Enhanced LCARS dashboard features:" -ForegroundColor Magenta
Write-Host "  - System Integrity card with status bar" -ForegroundColor Gray
Write-Host "  - 4-column metrics grid (Performance, Regression, Memory, Frames)" -ForegroundColor Gray
Write-Host "  - Test distribution by category" -ForegroundColor Gray
Write-Host "  - Collapsible test sections with auto-expand for failures" -ForegroundColor Gray
Write-Host "  - Complete flat test listing for reference" -ForegroundColor Gray
