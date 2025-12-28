using UnrealBuildTool;

public class FringeNetwork : ModuleRules
{
    public FringeNetwork(ReadOnlyTargetRules Target) : base(Target)
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
            "Projects",
            "InputCore",
            "Slate",
            "SlateCore"
        });

        // Define WITH_IMGUI to control optional ImGui functionality
        PublicDefinitions.Add("WITH_IMGUI=0");
    }
}
