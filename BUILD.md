# Building & Running NexusQA

This guide covers how to build the framework from source, run tests, and troubleshoot common issues.

## System Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| **Unreal Engine** | 5.0 | 5.6+ |
| **OS** | Windows 10, Ubuntu 20.04, macOS 12 | Windows 11, Ubuntu 22.04, macOS 13+ |
| **RAM** | 16 GB | 32 GB |
| **Disk** | 150 GB (Engine + project) | 250 GB |
| **GPU** | Integrated (Intel/AMD) | Dedicated (NVIDIA/AMD) |

## Quick Start

### Windows

```powershell
# 1. Clone the repo
git clone https://github.com/calionestevar/NexusQA
cd NexusQA

# 2. Run the test harness (no UE install required for demo)
.\Scripts\Engage.bat

# If you have Unreal Engine installed:
# - Set UE_ENGINE_PATH env var, or edit Engage.ps1 to specify engine path
```

### Linux / macOS

```bash
# 1. Clone the repo
git clone https://github.com/calionestevar/NexusQA
cd NexusQA

# 2. Run the test harness
./Scripts/RideOut.sh

# To use Legacy Asgard commandlet:
./Scripts/RideOut.sh --legacy
```

---

## Building from Source

### Prerequisites

1. **Download Unreal Engine 5.6+**
   ```bash
   # Via Epic Games Launcher, or:
   git clone --depth 1 --branch 5.6 https://github.com/EpicGames/UnrealEngine.git
   cd UnrealEngine
   ./Setup.sh  # or Setup.bat on Windows
   ./GenerateProjectFiles.sh
   make
   ```

2. **Set Engine Path (Optional)**
   ```powershell
   # Windows PowerShell
   $env:UE_ENGINE_PATH = "C:\Program Files\Epic Games\UE_5.6"
   
   # Linux/macOS bash
   export UE_ENGINE_PATH="/path/to/UnrealEngine"
   ```

### Generate Project Files

```bash
# Windows
"C:\Program Files\Epic Games\UE_5.6\Engine\Build\BatchFiles\Build.bat" \
  NexusDemo Win64 Development -Project="path\to\NexusDemo.uproject" \
  -TargetType=Editor -Progress

# Linux/macOS
$UE_ENGINE_PATH/Engine/Build/BatchFiles/Linux/Build.sh \
  NexusDemo Development -Project="$(pwd)/NexusDemo.uproject" \
  -TargetType=Editor -Progress
```

### Build the Project

**Option A: Command Line (Headless)**
```bash
# Windows
.\Scripts\Engage.ps1

# Linux/macOS
./Scripts/RideOut.sh
```

**Option B: Unreal Editor (GUI)**
```powershell
# Windows
& "C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" `
  "$(pwd)/NexusDemo.uproject"

# Linux/macOS
$UE_ENGINE_PATH/Engine/Binaries/Linux/UnrealEditor \
  "$(pwd)/NexusDemo.uproject"
```

Then in the Editor:
1. **Tools → Compile**
2. **File → Package Project** (if needed)

---

## Running Tests

### Understanding Test Frameworks

**Nexus vs Asgard:**

- **Nexus** (Recommended) — Modern, project-level test orchestration framework. Handles test discovery, parallel execution across worker processes, fail-fast on critical tests, test reporting, and integration with the Palantír observer for live analysis and final reports. Designed for CI, headless execution, and large-scale runs.

- **Asgard (Legacy)** — Engine-native commandlet-based approach that demonstrates knowledge of Unreal's AutomationTest and Commandlet patterns. Kept for compatibility and to show experience with standard engine tooling.

**Why both exist:** Asgard shows familiarity with engine-native testing patterns. Nexus is the canonical CI-level runner that provides higher-level orchestration, parallelism, and richer reporting.

### Full Test Suite

```powershell
# Windows (via Engage.ps1)
.\Scripts\Engage.ps1

# Linux/macOS (via RideOut.sh)
./Scripts/RideOut.sh
```

**What happens:**
1. Discovers all `NEXUS_TEST` macros
2. Runs tests in parallel workers
3. Exports results to `Saved/NexusReports/`
4. Opens LCARS HTML report (if available)

### Run Specific Tests

```powershell
# Windows
.\Scripts\Engage.ps1 -TestPattern "FringeNetwork.*"

# Linux/macOS
./Scripts/RideOut.sh -TestPattern "FringeNetwork.*"
```

### Run with Arguments

```powershell
# Windows - verbose logging
.\Scripts\Engage.ps1 -Verbose

# Linux/macOS - skip performance tests
./Scripts/RideOut.sh --skip-performance
```

### PowerShell Execution Policy

If you see "running scripts is disabled on this system" when invoking `Engage.ps1`, your PowerShell `ExecutionPolicy` is blocking script execution. Resolve it safely:

- **Easiest (no policy changes):** Run the batch wrapper:
  ```powershell
  .\Scripts\Engage.bat
  ```

- **Temporary for this session only:**
  ```powershell
  powershell -NoProfile -ExecutionPolicy Bypass -File .\Scripts\Engage.ps1
  ```

- **Persistent (user-scoped, no admin required):**
  ```powershell
  Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
  ```
  To revert later: `Set-ExecutionPolicy -ExecutionPolicy Restricted -Scope CurrentUser`

- **Corporate managed machines:** Run the `Engage.bat` wrapper or ask IT to permit script execution.

### Legacy Commandlet (Asgard)

```bash
./Scripts/RideOut.sh --legacy
```

---

## Test Artifacts

After running tests, check `Saved/NexusReports/`:

| File | Purpose | View With |
|------|---------|-----------|
| `LCARSReport.json` | Structured results | Text editor / jq |
| `LCARS_Report_*.html` | Visual dashboard | Web browser |
| `nexus-results.xml` | JUnit XML | CI tools |
| `ArgusLensPerformance.json` | Performance metrics | Text editor |
| `TransfigurationReport.json` | Accessibility audit | Text editor |

**Example:**
```bash
# View HTML report
open Saved/NexusReports/LCARS_Report_*.html  # macOS
start Saved/NexusReports/LCARS_Report_*.html # Windows
firefox Saved/NexusReports/LCARS_Report_*.html # Linux

# Parse JSON results
cat Saved/NexusReports/LCARSReport.json | jq '.tests[] | select(.status=="FAILED")'
```

---

## Troubleshooting

### Build Errors

#### "UnrealEditor-Cmd.exe not found"
```powershell
# Solution 1: Set UE_ENGINE_PATH
$env:UE_ENGINE_PATH = "C:\Program Files\Epic Games\UE_5.6"
.\Scripts\Engage.ps1

# Solution 2: Edit Engage.ps1 directly
# Change line: $EngineExe = ...
# to your engine path

# Solution 3: Use GUI Editor
"C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe" NexusDemo.uproject
```

#### "Module dependencies not found" / "Missing .Build.cs"
```powershell
# Regenerate project files
"$env:UE_ENGINE_PATH\Engine\Build\BatchFiles\GenerateProjectFiles.bat" -VSVersion=2022
```

#### Compilation errors on M1/M2 Mac
```bash
# Ensure you're using ARM64 build of UE
# Download from Epic Games Launcher → UE 5.6 → Platforms → macOS (ARM64)

# If building from source:
git checkout 5.6
./Setup.sh
./GenerateProjectFiles.sh
make -j$(sysctl -n hw.ncpu)
```

### Test Failures

#### Tests timeout or hang
```powershell
# Check if engine process is stuck
Get-Process UnrealEditor-Cmd -ErrorAction SilentlyContinue | Stop-Process -Force

# Run with verbose logging
.\Scripts\Engage.ps1 -Verbose
```

#### "No NEXUS_TEST found"
```
Ensure:
1. Tests are in Source/<Module>/Private/Tests/*.test.cpp
2. Tests use NEXUS_TEST() macro
3. #include "NexusTest.h" is present
4. Module is registered in NexusDemo.uproject
5. Build.cs includes NexusTest in dependencies
```

#### Performance metrics always 0
```
ArgusLens runs continuously in the background. If values are 0:
1. Ensure test duration is >100ms (default sample interval)
2. Check if nullrhi is enabled (disables GPU; some metrics unavailable)
3. Verify UArgusLens::StartPerformanceMonitoring() called
```

### CI/CD Issues

#### GitHub Actions fails with "Engine not found"
```yaml
# In .github/workflows/ci.yml, ensure:
- name: Set UE_ENGINE_PATH
  run: echo "UE_ENGINE_PATH=C:\Program Files\Epic Games\UE_5.6" >> $GITHUB_ENV
  shell: pwsh
```

#### Artifacts not uploaded
```yaml
# Check paths match:
- name: Upload artifacts
  uses: actions/upload-artifact@v4
  with:
    path: Saved/NexusReports/**  # Must exist after tests
```

---

## Development Workflow

### Adding a Test

1. **Create test file:**
   ```cpp
   // Source/FringeNetwork/Private/Tests/MyTest.test.cpp
   #include "NexusTest.h"
   #include "CortexiphanInjector.h"
   
   NEXUS_TEST(FMyFringeTest, "FringeNetwork.MyFeature.Scenario", ETestPriority::Normal)
   {
       UCortexiphanInjector::InjectChaos(10.0f, 0.5f);
       return true;
   }
   ```

2. **Rebuild and run:**
   ```powershell
   .\Scripts\Engage.ps1
   ```

3. **Check output:**
   ```
   Saved/NexusReports/LCARS_Report_*.html
   ```

### Enabling Custom Modules

To add a new testing module (e.g., "MyFramework"):

1. **Create folder structure:**
   ```bash
   mkdir -p Source/MyFramework/{Public,Private/Tests}
   touch Source/MyFramework/MyFramework.Build.cs
   ```

2. **Add Build.cs:**
   ```csharp
   using UnrealBuildTool;
   
   public class MyFramework : ModuleRules
   {
       public MyFramework(ReadOnlyTargetRules Target) : base(Target)
       {
           PublicDependencyModuleNames.AddRange(new string[] {
               "Core", "CoreUObject", "Engine"
           });
       }
   }
   ```

3. **Register in NexusDemo.uproject:**
   ```json
   {
       "Name": "MyFramework",
       "Type": "Runtime",
       "LoadingPhase": "Default"
   }
   ```

4. **Regenerate project files and rebuild**

---

## Performance Tips

- **Run tests in parallel:** Default behavior; use `-Sequential` flag to disable
- **Skip slow tests:** Mark with `ETestPriority::Slow`; exclude with `-SkipSlow`
- **Use nullrhi for headless:** Faster when no GPU output needed (default for CI)
- **Enable shader caching:** Reuses compiled shaders across builds

---

## Further Reading

- [docs/modules.md](docs/modules.md) — Module reference
- [docs/TestingFramework.md](docs/TestingFramework.md) — Writing tests
- [CONTRIBUTING.md](CONTRIBUTING.md) — Development standards
- [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) — Design deep-dive
