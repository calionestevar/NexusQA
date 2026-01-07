using UnrealBuildTool;

public class Nexus : ModuleRules
{
    public Nexus(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.AddRange(new string[]
        {
            "Nexus/Public"
        });

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
            "JsonUtilities",
            "Sockets",
            "Networking"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "Projects",
            "InputCore",
            "Slate",
            "SlateCore"
        });

        // ImGui is optional for live overlay support
        // Uncomment below if you have ImGui plugin installed
        // PrivateDependencyModuleNames.Add("ImGui");

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
    }
}