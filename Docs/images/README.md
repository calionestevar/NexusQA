# Screenshot Capture Guide

This directory contains visual assets showcasing NexusQA in action.

## Status: LCARS Dashboard Ready for Screenshots! 📸

The enhanced HTML dashboard is complete. Generate it with:
```powershell
.\Scripts\Generate-LCARSReport.ps1
```
Opens `TestReports/LCARS_Demo_Report.html` - ready for capture!

## Planned Screenshots

### 1. LCARS HTML Dashboard
**File:** `lcars-dashboard.png`
- Full test report with Starfleet theme
- Pass/fail summary section
- Performance metrics graphs
- Test details table
- Artifact links

### 2. API Testing with Trace Correlation
**File:** `api-tracing.png`
- Side-by-side view of:
  - Unreal Engine test code
  - HTTP request with X-Trace-ID header
  - PlayFab logs showing same trace ID
  - Sentry error report tagged with trace ID

### 3. Performance Monitoring
**File:** `argus-lens-dashboard.png`
- FPS graph over time
- Memory usage chart
- Frame time distribution
- Hitch detection markers

### 4. Distributed Tracing Timeline
**File:** `trace-timeline.png`
- JSON breadcrumb export
- Visual timeline showing:
  - Test start
  - HTTP requests
  - Backend processing
  - Test completion

### 5. CI/CD Integration
**File:** `github-actions.png`
- GitHub Actions workflow running
- Test summary in PR comments
- Artifact uploads (HTML, XML, JSON)
- Pass/fail badge

### 6. Code Examples
**File:** `code-examples.png`
- Split view of test code showing:
  - Automatic trace injection
  - Fluent API usage
  - Rich assertions with context

## How to Add Screenshots

1. Run tests and generate artifacts
2. Take screenshots of key features
3. Save as PNG files with descriptive names
4. Update README.md to reference images
5. Commit and push to repository

## Image Guidelines

- **Format:** PNG (lossless, supports transparency)
- **Resolution:** 1920x1080 or higher
- **File size:** Keep under 1MB if possible (optimize with tools like TinyPNG)
- **Naming:** Use kebab-case (e.g., `lcars-dashboard.png`)
- **Alt text:** Always provide descriptive alt text in markdown

## Example Usage in Markdown

```markdown
![LCARS Dashboard](docs/images/lcars-dashboard.png)
*Starfleet-themed HTML test report showing 45/50 tests passed with performance metrics*
```
