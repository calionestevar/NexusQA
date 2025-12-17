using UnrealBuildTool;

public class Nexus : ModuleRules
{
    public Nexus(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(new string[] 
        { 
            "Nexus/Core/Public",
            "Nexus/Core/Private",
            "Nexus/LCARSBridge/Public",
            "Nexus/LCARSBridge/Private",
            "Nexus/Palantir/Public",
            "Nexus/Palantir/Private",
            "Nexus/Private"
        });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "HTTP",
            "Json",
            "JsonUtilities"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Projects",
            "InputCore",
            "Slate",
            "SlateCore"
        });

        // ImGui is optional - only include if available
        if (Target.IsInPlugin("ImGui") || System.IO.Directory.Exists(System.IO.Path.Combine(Target.ProjectDir, "Plugins/ImGui")))
        {
            PrivateDependencyModuleNames.Add("ImGui");
        }

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}