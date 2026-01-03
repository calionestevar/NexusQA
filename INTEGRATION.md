# NexusQA Integration Checklist

> **Latest Updates (Jan 2, 2026):** Auto-PIE world detection, enhanced LCARS HTML reporting with Star Trek-inspired styling, and fixed encoding issues in logging systems.

Before integrating NexusQA into your demo game, verify the following.

> **After integration is complete**, see [Docs/INTEGRATION_GUIDE.md](Docs/INTEGRATION_GUIDE.md) for guidance on using multiple modules together, choosing between NEXUS_TEST and NEXUS_TEST_GAMETHREAD, and avoiding common pitfalls.

## ✅ Prerequisites
- [ ] Your game project uses Unreal Engine 5.7+
- [ ] You have git installed (for submodule integration)
- [ ] Your project compiles cleanly on current engine version

## 🔧 Integration Steps

### 1. Add as Git Submodule
```bash
cd YourGameProject
git submodule add https://github.com/calionestevar/NexusQA.git Plugins/NexusQA
git commit -m "Add NexusQA testing framework"
```

### 2. Regenerate Project Files
```bash
# Close the project in Editor
# Windows
.\GenerateProjectFiles.bat
# macOS/Linux
./GenerateProjectFiles.sh
```

### 3. Rebuild Solution
- Open the solution file in Visual Studio
- Build the solution (Ctrl+Shift+B)
- Wait for compilation to complete

### 4. Open in Editor
- Launch your game project in UE5
- Editor should detect and load the NexusQA plugin
- Check Plugins window (Ctrl+Shift+P) to verify modules loaded

## ⚙️ Module Dependencies
NexusQA requires these engine modules (should be available in all UE5.6+ projects):
- Core
- CoreUObject
- Engine
- HTTP
- Json
- JsonUtilities
- InputCore
- Projects

If you have compilation errors, verify these modules are enabled in your project.

## 🔄 UE 5.7 Compatibility

NexusQA is fully compatible with Unreal Engine 5.7+. Recent updates include:

**API Fixes Applied:**
- ✅ Fixed `bIsEditorWorld` deprecation (now uses `WorldType != EWorldType::Editor`)
- ✅ Fixed `ANY_PACKAGE` macro removal (now uses `nullptr`)
- ✅ Added explicit type casting for ACharacter→AActor conversion
- ✅ Included GameFramework/Character.h for full type definitions

**Game-World Auto-Detection:**
- Game-thread tests automatically detect if PIE world is available
- If no world is found, tests gracefully skip with helpful user guidance
- Logs suggest clicking "Play" in editor to enable full world context

No additional configuration needed - just add NexusQA as a submodule and build!

## 🧪 Testing Integration
Once integrated, try running a simple test:
```cpp
#include "Nexus/Public/NexusTest.h"

IMPLEMENT_NEXUS_TEST(FSimpleTest)
{
    return true; // This test always passes
}
```

## ✨ New Features: Foundation + Quick-Wins

### FNexusTestContext - World Access
Tests can now access the game world, game state, and player controller:
```cpp
NEXUS_TEST_GAMETHREAD(FMyGameTest, "MyGame.WorldAccess", ETestPriority::Normal)
{
    if (!Context.IsValid())
    {
        return true;  // World not available (OK in some scenarios)
    }
    
    // Access world directly
    ACharacter* TestChar = Context.SpawnTestCharacter(MyCharClass, FVector(0, 0, 0));
    
    // Actors automatically cleaned up on test end via RAII
    return TestChar != nullptr;
}
```

### Skip/Conditional Execution
Skip tests based on conditions:
```cpp
NEXUS_TEST(FConditionalTest, "MyGame.ConditionalSkip", ETestPriority::Normal)
{
    // Skip on certain platforms or configurations
    // bSkip = FPlatformProperties::IsServerOnly();
    
    return true;
}
```

### Automated Retry Logic
Automatically retry flaky tests with exponential backoff:
```cpp
class FFlakeyNetworkTest : public FNexusTest
{
public:
    FFlakeyNetworkTest() : FNexusTest(...)
    {
        MaxRetries = 3;  // Retry up to 3 times with 1s, 2s, 4s delays
    }
    bool RunTest(const FNexusTestContext& Context);
};
```

### Performance Monitoring with ArgusLens
Monitor FPS, memory, and hitches during test execution:
```cpp
NEXUS_PERF_TEST(FPerformanceTest, "Perf.Rendering.60FPS", ETestPriority::Normal, 60.0f)
{
    // Test runs for 60 seconds with ArgusLens monitoring
    
    if (HAS_PERF_DATA(Context))
    {
        // Assert performance gates
        ASSERT_AVERAGE_FPS(Context, 60.0f);      // Minimum 60 FPS
        ASSERT_MAX_MEMORY(Context, 2048.0f);     // Maximum 2GB memory
        ASSERT_MAX_HITCHES(Context, 5);          // No more than 5 frame hitches
    }
    
    return true;
}
```

**Performance Assertion Helpers:**
- `ASSERT_AVERAGE_FPS(Context, MinFPS)` - Verify minimum average FPS
- `ASSERT_MAX_MEMORY(Context, MaxMb)` - Verify memory stays under limit
- `ASSERT_MAX_HITCHES(Context, MaxCount)` - Verify hitch count stays low
- `HAS_PERF_DATA(Context)` - Check if performance data is available

## 📚 Next Steps
- Read [PORTFOLIO.md](../PORTFOLIO.md) for architecture overview
- Check [Docs/INTEGRATION_GUIDE.md](../Docs/INTEGRATION_GUIDE.md) for complete patterns and pitfalls
- See [README.md](../README.md) for comprehensive documentation

## 🆘 Troubleshooting

**Plugin not loading?**
- Verify NexusQA.uplugin exists in Plugins/NexusQA/
- Check Engine version matches (5.7.0+)
- Regenerate project files and rebuild

**Compilation errors?**
- Ensure all required modules are present
- Check that your game .Build.cs files reference NexusQA modules
- Clean and rebuild solution
- Verify GameFramework module is included in your project

**Tests not discovered?**
- Verify test macros use NEXUS_TEST, NEXUS_TEST_GAMETHREAD, or NEXUS_PERF_TEST
- Check that test code compiles without errors
- See Docs/TestingFramework.md for examples

**Game-thread tests have no world context?**
- This is normal when running in headless/command-line mode
- Click "Play" in the editor to start PIE mode before running tests
- Check logs for "No active game world detected" warning
- Performance tests and assertions gracefully skip without world context

**Performance assertions always pass?**
- Verify ArgusLens module is loaded (optional dependency)
- Make sure `NEXUS_PERF_TEST` macro is used instead of `NEXUS_TEST`
- Check that tests run long enough for metrics to be collected
- Use `HAS_PERF_DATA(Context)` to verify if metrics are available
