# Contributing to NexusQA

Thank you for your interest in contributing! This framework is designed to evolve with real-world QA demands, and community contributions are essential.

## Getting Started

1. **Fork & Clone**
   ```bash
   git clone https://github.com/YOUR_USERNAME/NexusQA.git
   cd NexusQA
   ```

2. **Set Up Dev Environment**
   ```powershell
   # Windows
   .\Scripts\Engage.bat
   
   # Linux/macOS
   ./Scripts/RideOut.sh
   ```

3. **Install Optional Dev Tools**
   ```powershell
   .\Scripts\install-imgui.ps1
   .\Scripts\install-tokra.bat  # or install-tokra.sh on Unix
   ```

## Code Standards

### C++ Style
- **Naming:** Follow Unreal conventions (UClassName for objects, FStructName for structs)
- **Headers:** Public API in `Public/`, implementation details in `Private/`
- **Logging:** Use module-specific log categories (e.g., `LogArgusLens`, `LogChaos`)
- **Comments:** Explain *why*, not *what*; code should be self-documenting

### File Organization
```
Source/ModuleName/
  ├─ Public/
  │  └─ *.h (public API)
  ├─ Private/
  │  ├─ *.cpp (implementation)
  │  └─ Tests/
  │     └─ *.test.cpp (unit/integration tests)
  └─ ModuleName.Build.cs
```

### Testing Requirements
- **New features** must include tests in `Source/<Module>/Private/Tests/`
- **Test naming:** Use `NEXUS_TEST` macro with descriptive names
  ```cpp
  NEXUS_TEST(FMyFeature, "Module.Feature.Scenario", ETestPriority::Normal)
  {
      // Test logic
  }
  ```
- **Coverage target:** Aim for >80% on new code paths
- **Run before commit:** `.\Scripts\Engage.ps1` (Windows) or `./Scripts/RideOut.sh` (Unix)

## Branching & Commits

1. **Create a feature branch**
   ```bash
   git checkout -b feature/my-awesome-feature
   ```

2. **Commit messages** should be clear and concise
   ```
   [Module] Brief description
   
   Longer explanation if needed. Reference issues: Fixes #123
   ```

3. **Push & Create PR**
   ```bash
   git push origin feature/my-awesome-feature
   ```

## Pull Request Process

1. **Update docs** if you change behavior or add modules
2. **Run full test suite** (`Engage.ps1` / `RideOut.sh`)
3. **Check CI passes** (GitHub Actions must be green)
4. **Keep PR focused** — one feature per PR; large changes → multiple PRs
5. **Respond to reviews** promptly; be respectful & collaborative

## Adding a New Module

If you're adding a new testing feature:

1. **Create the module structure**
   ```bash
   mkdir -p Source/MyModule/{Public,Private/Tests}
   touch Source/MyModule/MyModule.Build.cs
   ```

2. **Add module entry** to `NexusDemo.uproject`
   ```json
   {
       "Name": "MyModule",
       "Type": "Runtime",
       "LoadingPhase": "Default"
   }
   ```

3. **Create sample tests** in `Private/Tests/`

4. **Update docs** — add description to `docs/modules.md`

5. **Wire dependencies** in `MyModule.Build.cs`

## Reporting Issues

- **Bug reports:** Include Unreal version, OS, reproduction steps
- **Feature requests:** Explain the use case and why it matters for QA
- **Questions:** Check existing issues/docs first; open a Discussion if it's not a bug

## Code Review Checklist

Before submitting a PR, ensure:
- ✅ Code follows C++ style guidelines
- ✅ Tests added/updated with >80% coverage
- ✅ `Engage.ps1` / `RideOut.sh` runs without new errors
- ✅ Documentation (README, module docs, ARCHITECTURE) updated
- ✅ No circular module dependencies
- ✅ Commit messages are clear

## Questions?

- **Architecture:** See [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md)
- **Modules:** See [docs/modules.md](docs/modules.md)
- **Testing:** See [docs/TestingFramework.md](docs/TestingFramework.md)
- **Reporting:** See [docs/LCARS.md](docs/LCARS.md)

Welcome aboard! 🚀
