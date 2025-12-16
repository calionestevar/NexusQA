# NexusQA Integration Checklist

Before integrating NexusQA into your demo game, verify the following:

## ✅ Prerequisites
- [ ] Your game project uses Unreal Engine 5.6+
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

## 🧪 Testing Integration
Once integrated, try running a simple test:
```cpp
#include "Nexus/Public/NexusTest.h"

IMPLEMENT_NEXUS_TEST(FSimpleTest)
{
    return true; // This test always passes
}
```

## 📚 Next Steps
- Read [PORTFOLIO.md](../PORTFOLIO.md) for architecture overview
- Check [Docs/API_TESTING.md](../Docs/API_TESTING.md) for API testing examples
- See [README.md](../README.md) for comprehensive documentation

## 🆘 Troubleshooting

**Plugin not loading?**
- Verify NexusQA.uplugin exists in Plugins/NexusQA/
- Check Engine version matches (5.6.0+)
- Regenerate project files and rebuild

**Compilation errors?**
- Ensure all required modules are present
- Check that your game .Build.cs files reference NexusQA modules
- Clean and rebuild solution

**Tests not discovered?**
- Verify test macros use IMPLEMENT_NEXUS_TEST correctly
- Check that test code compiles without errors
- See Docs/TestingFramework.md for examples
