# Protego — Compliance & Accessibility Testing

**Protego** provides automated compliance checking for regulations (COPPA, GDPR, DSA), accessibility validation (color-blindness, subtitles), and safety system enforcement. Named after the protection charm from Harry Potter, it safeguards your game against legal, ethical, and accessibility violations.

---

## Overview

### Core Capabilities

| Feature | Purpose | Use Case |
|---------|---------|----------|
| **COPPA Compliance** | Verify age-appropriate content | Ensure compliance with Children's Online Privacy Protection Act |
| **GDPR Validation** | Check PII handling | Validate personal data protection and consent |
| **DSA Compliance** | EU Digital Services Act checks | Content moderation, transparency requirements |
| **Color-Blindness Testing** | Validate color contrast | Ensure users with color vision deficiency see UI clearly |
| **Subtitle/Captions** | Accessibility for deaf/HoH | Verify subtitles exist and sync correctly |
| **Safety System Audit** | Verify safety rules enforced | Ensure guard rails prevent harmful content |

---

## Quick Start

### 1. Add Protego to Your Project

**In your `.Build.cs` file:**

```csharp
PublicDependencyModuleNames.AddRange(new string[] {
    "Protego",
    "Json",
    "JsonUtilities"
});
```

### 2. Load Compliance Rules

```cpp
#include "Protego/Public/Protego.h"

void AMyGameMode::BeginPlay()
{
    Super::BeginPlay();
    
    // Load compliance rules from JSON
    bool bLoaded = UProtego::LoadComplianceRules(TEXT("Protego/ComplianceRules.json"));
    
    if (!bLoaded) {
        UE_LOG(LogTemp, Error, TEXT("Failed to load compliance rules!"));
    }
}
```

### 3. Run Compliance Checks

```cpp
void AMyGameMode::ValidateCompliance()
{
    // Check COPPA requirements
    if (!UProtego::CheckCOPPA(CurrentUserProfile)) {
        UE_LOG(LogTemp, Error, TEXT("❌ COPPA validation failed"));
        return;
    }
    
    // Check GDPR requirements
    if (!UProtego::CheckGDPR(CurrentUserData)) {
        UE_LOG(LogTemp, Error, TEXT("❌ GDPR validation failed"));
        return;
    }
    
    // Check accessibility
    if (!UProtego::ValidateColorBlindness()) {
        UE_LOG(LogTemp, Warning, TEXT("⚠️  Color-blindness accessibility issues detected"));
    }
    
    UE_LOG(LogTemp, Display, TEXT("✅ All compliance checks passed"));
}
```

---

## API Reference

### Load Compliance Rules

```cpp
bool UProtego::LoadComplianceRules(const FString& RulesFilePath);
```

Loads compliance rule definitions from JSON file.

**Parameters:**
- `RulesFilePath` — Path to rules JSON (e.g., `"Protego/ComplianceRules.json"`)

**Example:**

```cpp
// Load default rules
if (!UProtego::LoadComplianceRules(TEXT("Protego/ComplianceRules.json"))) {
    UE_LOG(LogTemp, Error, TEXT("Failed to load compliance rules"));
    return false;
}

UE_LOG(LogTemp, Display, TEXT("✅ Compliance rules loaded"));
```

**Rules File Format:**

```json
{
  "COPPA": {
    "MinAgeUnprotected": 13,
    "RequireParentalConsentUnder": 13,
    "AllowedContentRatings": ["E", "E10+"],
    "ProhibitedContentTypes": ["Violence", "StrongLanguage", "Ads"]
  },
  "GDPR": {
    "RequireExplicitConsent": true,
    "AllowAutomatic PII": false,
    "MustAllowDataDeletion": true,
    "MaxDataRetentionDays": 365
  },
  "DSA": {
    "MustModeratePII": true,
    "RequireTransparencyReports": true,
    "MaxHarmfulContentPercent": 0.1
  }
}
```

---

### COPPA Compliance Check

```cpp
bool UProtego::CheckCOPPA(const FUserProfile& UserProfile);
```

Validates compliance with Children's Online Privacy Protection Act.

**What it checks:**
- User age vs. content rating
- Parental consent for users under 13
- No collection of unnecessary PII from minors
- Age-appropriate content delivery

**Example:**

```cpp
NEXUS_TEST(FCOPPAComplianceTest, "Compliance.COPPA.AgeGating", ETestPriority::Critical)
{
    // Test 1: Adult (18+) can access all content
    FUserProfile AdultUser;
    AdultUser.Age = 25;
    AdultUser.HasParentalConsent = false;
    
    if (!UProtego::CheckCOPPA(AdultUser)) {
        UE_LOG(LogTemp, Error, TEXT("❌ Adult user COPPA check failed"));
        return false;
    }
    
    // Test 2: Child (under 13) without consent is restricted
    FUserProfile ChildUser;
    ChildUser.Age = 10;
    ChildUser.HasParentalConsent = false;
    
    // Should fail without consent
    if (UProtego::CheckCOPPA(ChildUser)) {
        UE_LOG(LogTemp, Error, TEXT("❌ Child without consent passed (security issue!)"));
        return false;
    }
    
    // Test 3: Child WITH consent is allowed
    ChildUser.HasParentalConsent = true;
    if (!UProtego::CheckCOPPA(ChildUser)) {
        UE_LOG(LogTemp, Error, TEXT("❌ Child with valid consent failed"));
        return false;
    }
    
    return true;
}
```

---

### GDPR Compliance Check

```cpp
bool UProtego::CheckGDPR(const FUserData& UserData);
```

Validates Personal Data protection and GDPR compliance.

**What it checks:**
- User has given explicit consent for data collection
- PII isn't unnecessarily retained
- Users can request data deletion
- Data isn't sold to third parties without consent

**Example:**

```cpp
NEXUS_TEST(FGDPRComplianceTest, "Compliance.GDPR.DataProtection", ETestPriority::Critical)
{
    // Test 1: Collecting PII without consent is blocked
    FUserData UserWithoutConsent;
    UserWithoutConsent.bHasCollectConsent = false;
    UserWithoutConsent.PII = FString("name=John&email=john@example.com");
    
    if (UProtego::CheckGDPR(UserWithoutConsent)) {
        UE_LOG(LogTemp, Error, TEXT("❌ PII collected without consent!"));
        return false;
    }
    
    // Test 2: Valid consent allows data collection
    FUserData UserWithConsent;
    UserWithConsent.bHasCollectConsent = true;
    UserWithConsent.PII = FString("name=John&email=john@example.com");
    
    if (!UProtego::CheckGDPR(UserWithConsent)) {
        UE_LOG(LogTemp, Error, TEXT("❌ Valid consent not recognized"));
        return false;
    }
    
    // Test 3: Data deletion is enabled
    if (!UProtego::AllowUserDataDeletion(UserWithConsent.UserID)) {
        UE_LOG(LogTemp, Error, TEXT("❌ User data deletion not allowed"));
        return false;
    }
    
    return true;
}
```

---

### DSA Compliance Check

```cpp
bool UProtego::CheckDSA(const FContentModerationResult& ModerationResult);
```

Validates EU Digital Services Act compliance.

**What it checks:**
- Content moderation is working
- Harmful content is removed/flagged
- Transparency reports generated
- Algorithm explainability documented

**Example:**

```cpp
FContentModerationResult Result;
Result.HarmfulContentDetected = 5;  // 5 flagged items
Result.TotalContentItems = 1000;
Result.ModerationAccuracy = 0.98f;

if (!UProtego::CheckDSA(Result)) {
    UE_LOG(LogTemp, Error, TEXT("❌ DSA moderation thresholds exceeded"));
    return false;
}
```

---

### Color-Blindness Validation

```cpp
bool UProtego::ValidateColorBlindness(
    EColorBlindType BlindnessType = EColorBlindType::Deuteranopia
);
```

Checks UI is visible for users with color vision deficiency.

**Supported Types:**
- `Deuteranopia` — Red-green color blindness (most common)
- `Protanopia` — Red color blindness
- `Tritanopia` — Blue-yellow color blindness

**Example:**

```cpp
NEXUS_TEST(FColorBlindnessTest, "Accessibility.Colorblind.UIContrast", ETestPriority::High)
{
    // Test each color blindness type
    TArray<EColorBlindType> BlindnessTypes = {
        EColorBlindType::Deuteranopia,
        EColorBlindType::Protanopia,
        EColorBlindType::Tritanopia
    };
    
    for (EColorBlindType Type : BlindnessTypes) {
        if (!UProtego::ValidateColorBlindness(Type)) {
            UE_LOG(LogTemp, Error, TEXT("❌ UI not accessible for %s"), 
                *EnumToString(Type));
            return false;
        }
    }
    
    UE_LOG(LogTemp, Display, TEXT("✅ All color blindness validations passed"));
    return true;
}
```

**What it checks:**
- Buttons use text labels, not just colors
- Status indicators use shapes/patterns, not just colors
- Chart colors are distinguishable
- Minimum contrast ratio met (4.5:1 for text)

---

### Subtitle/Caption Validation

```cpp
bool UProtego::ValidateSubtitles(
    ELanguage Language = ELanguage::English,
    EAccessibilityStandard Standard = EAccessibilityStandard::WCAG2_1_AA
);
```

Checks subtitles exist and meet accessibility standards.

**Parameters:**
- `Language` — Which language subtitle to validate
- `Standard` — Accessibility standard (WCAG 2.1 AA, WCAG 2.1 AAA)

**Example:**

```cpp
NEXUS_TEST(FSubtitleAccessibilityTest, "Accessibility.Subtitles.Coverage", ETestPriority::High)
{
    // Check English subtitles meet WCAG AA standard
    if (!UProtego::ValidateSubtitles(ELanguage::English, EAccessibilityStandard::WCAG2_1_AA)) {
        UE_LOG(LogTemp, Error, TEXT("❌ English subtitles don't meet WCAG AA"));
        return false;
    }
    
    // Check Spanish subtitles
    if (!UProtego::ValidateSubtitles(ELanguage::Spanish, EAccessibilityStandard::WCAG2_1_AA)) {
        UE_LOG(LogTemp, Error, TEXT("❌ Spanish subtitles don't meet WCAG AA"));
        return false;
    }
    
    return true;
}
```

**What it checks:**
- All dialog has subtitles
- Sound effects are captioned (e.g., "[gunshot]")
- Subtitles are synchronized within 100ms
- Readable font size (minimum 18pt)
- High contrast with background

---

### Safety System Validation

```cpp
bool UProtego::ValidateSafetySystemEnforced();
```

Verifies that safety rules are properly enforced.

**Example:**

```cpp
void AMyGameMode::ValidateSafety()
{
    if (!UProtego::ValidateSafetySystemEnforced()) {
        UE_LOG(LogTemp, Error, TEXT("❌ Safety system not properly enforced!"));
        GetWorld()->RequestEndGame(true);
        return;
    }
    
    UE_LOG(LogTemp, Display, TEXT("✅ Safety system validated"));
}
```

**What it checks:**
- Dangerous patterns are blocked (null dereferences, etc.)
- Resource limits enforced
- No undefined behavior
- Memory access is safe

---

## Integration Examples

### Example 1: Pre-Launch Compliance Audit

```cpp
// ComplianceAuditTest.cpp
NEXUS_TEST(FComplianceAuditTest, "Compliance.PreLaunch.FullAudit", ETestPriority::Critical)
{
    // Load rules
    if (!UProtego::LoadComplianceRules(TEXT("Protego/ComplianceRules.json"))) {
        return false;
    }
    
    bool bAllPassed = true;
    
    // COPPA test
    {
        FUserProfile TestUser;
        TestUser.Age = 10;
        TestUser.HasParentalConsent = true;
        
        if (!UProtego::CheckCOPPA(TestUser)) {
            UE_LOG(LogTemp, Error, TEXT("❌ COPPA check failed"));
            bAllPassed = false;
        }
    }
    
    // GDPR test
    {
        FUserData TestData;
        TestData.bHasCollectConsent = true;
        
        if (!UProtego::CheckGDPR(TestData)) {
            UE_LOG(LogTemp, Error, TEXT("❌ GDPR check failed"));
            bAllPassed = false;
        }
    }
    
    // Accessibility tests
    if (!UProtego::ValidateColorBlindness()) {
        UE_LOG(LogTemp, Error, TEXT("❌ Color-blindness validation failed"));
        bAllPassed = false;
    }
    
    if (!UProtego::ValidateSubtitles(ELanguage::English)) {
        UE_LOG(LogTemp, Error, TEXT("❌ Subtitle validation failed"));
        bAllPassed = false;
    }
    
    // Safety validation
    if (!UProtego::ValidateSafetySystemEnforced()) {
        UE_LOG(LogTemp, Error, TEXT("❌ Safety system not enforced"));
        bAllPassed = false;
    }
    
    if (bAllPassed) {
        UE_LOG(LogTemp, Display, TEXT("✅ ALL COMPLIANCE CHECKS PASSED"));
    }
    
    return bAllPassed;
}
```

---

### Example 2: COPPA Age-Gating

```cpp
// AgeGatingTest.cpp
void UPlayerRegistration::RegisterPlayer(const FUserProfile& Profile)
{
    // Load compliance rules
    UProtego::LoadComplianceRules(TEXT("Protego/ComplianceRules.json"));
    
    // Check COPPA
    if (!UProtego::CheckCOPPA(Profile)) {
        if (Profile.Age < 13) {
            // Offer parental consent process
            ShowParentalConsentFlow(Profile);
            return;
        } else {
            // Age gating failed for other reason
            ShowComplianceErrorDialog();
            return;
        }
    }
    
    // User cleared compliance, allow registration
    FinalizePlayerRegistration(Profile);
}
```

---

### Example 3: Accessibility Pre-Launch

```cpp
// AccessibilityAuditTest.cpp
NEXUS_TEST(FAccessibilityAuditTest, "Accessibility.PreLaunch", ETestPriority::High)
{
    TArray<EColorBlindType> ColorTypes = {
        EColorBlindType::Deuteranopia,
        EColorBlindType::Protanopia,
        EColorBlindType::Tritanopia
    };
    
    TArray<ELanguage> SupportedLanguages = {
        ELanguage::English,
        ELanguage::Spanish,
        ELanguage::French,
        ELanguage::German
    };
    
    // Test color blindness for all UI
    for (EColorBlindType Type : ColorTypes) {
        if (!UProtego::ValidateColorBlindness(Type)) {
            UE_LOG(LogTemp, Error, TEXT("❌ Color-blind accessibility failed for type %d"), 
                static_cast<int32>(Type));
            return false;
        }
    }
    
    // Test subtitles for all languages
    for (ELanguage Lang : SupportedLanguages) {
        if (!UProtego::ValidateSubtitles(Lang)) {
            UE_LOG(LogTemp, Error, TEXT("❌ Subtitles not available for language %d"), 
                static_cast<int32>(Lang));
            return false;
        }
    }
    
    UE_LOG(LogTemp, Display, TEXT("✅ Accessibility audit PASSED"));
    return true;
}
```

---

## Configuration

### Compliance Rules JSON

**File: `Content/Protego/ComplianceRules.json`**

```json
{
  "COPPA": {
    "Enabled": true,
    "MinAgeUnprotected": 13,
    "AllowedContentRatings": ["E", "E10+"],
    "ProhibitedContentTypes": ["Violence", "StrongLanguage", "OnlineInteraction"],
    "RestrictedFeatures": ["SocialFeatures", "ChatWithStrangers"]
  },
  
  "GDPR": {
    "Enabled": true,
    "JurisdictionCoverage": ["EU", "UK"],
    "RequireExplicitConsent": true,
    "MaxDataRetentionDays": 365,
    "AllowAutomaticProcessing": false
  },
  
  "DSA": {
    "Enabled": true,
    "MustModeratePII": true,
    "AllowedHarmfulContentPercent": 0.01,
    "RequireTransparencyReports": true,
    "ModerationStandard": "MODERATE"
  },
  
  "Accessibility": {
    "ColorBlindnessSupport": true,
    "MinContrastRatio": 4.5,
    "SubtitleRequirement": "WCAG2_1_AA",
    "MinFontSize": 14
  }
}
```

---

## Troubleshooting

### Rules File Not Found

**Problem:** `LoadComplianceRules()` returns false

**Solutions:**
1. Check file path is correct
2. Verify file exists in `Content/Protego/`
3. Ensure JSON syntax is valid
4. Check file permissions

### Color Blindness Test Passes But UI Looks Wrong

**Problem:** Automated test passes but actual color-blind users report issues

**Solutions:**
1. Test with actual color-blind users (best validation)
2. Use online color blindness simulators
3. Check minimum contrast ratio is actually 4.5:1
4. Verify shape differentiation, not just colors

### Subtitles Missing for Some Languages

**Problem:** `ValidateSubtitles()` fails for one language

**Solutions:**
1. Verify localization files exist for that language
2. Check subtitles are properly synced
3. Ensure font supports that language's characters
4. Rebuild localization data

---

## See Also

- [OBSERVER_NETWORK_GUIDE.md](OBSERVER_NETWORK_GUIDE.md) — Log safety events
- [Nexus Core Framework](./NEXUS_GUIDE.md)
- [StargateStress Load Testing](./STARGATESTRESS_GUIDE.md)
