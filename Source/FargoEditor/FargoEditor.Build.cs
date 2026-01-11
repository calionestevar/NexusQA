using UnrealBuildTool;

public class FargoEditor : ModuleRules
{
    public FargoEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "UnrealEd",   // GEditor, PIE, etc.
            "Nexus"       // Core runtime module
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Slate",
            "SlateCore",
            "EditorSubsystem"
        });
    }
}