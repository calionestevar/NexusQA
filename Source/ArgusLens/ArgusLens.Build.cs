using UnrealBuildTool;

public class ArgusLens : ModuleRules
{
    public ArgusLens(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "Json",
            "JsonUtilities",
            "Nexus"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
        });
    }
}
