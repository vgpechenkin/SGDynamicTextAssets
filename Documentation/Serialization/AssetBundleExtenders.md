# Asset Bundle Extenders

[Back to Table of Contents](../TableOfContents.md)

## Overview

Asset bundle extenders control how [asset bundle](../Core/AssetBundles.md) metadata is extracted from DTA objects, written into serialized files, and read back during deserialization. They are the first concrete extender type in the [Serializer Extender Framework](SerializerExtenders.md).

**Base class:** `USGDTAAssetBundleExtender` (inherits `USGDTASerializerExtenderBase`)

**Header:** `SGDynamicTextAssetsRuntime/Public/Serialization/AssetBundleExtenders/SGDTAAssetBundleExtender.h`

**Constraints:**
- One asset bundle extender per serializer format (required)
- Instanced once per resolved class (not CDO), allowing state across a serialization process
- Blueprintable - developers can override behavior in Blueprint

## Three-Layer API

The extender uses a three-layer invocation pattern with five virtual methods at each layer:

| Layer | Visibility | Purpose |
|-------|-----------|---------|
| `Notify` | Public, non-virtual | Entry point called by serializer pipeline. Validates inputs, calls Native_ then BP_. |
| `Native_` | Protected, virtual | Core C++ behavior. Subclass in C++ and override for full control. |
| `BP_` | Protected, BlueprintNativeEvent | Blueprint-overridable hook. Called after Native_. Supplements C++ behavior. |

| Method | Purpose |
|--------|---------|
| `ExtractBundles` | Extract bundle metadata from a provider UObject |
| `PreSerialize` | Pre-processing before the serializer writes content |
| `PostSerialize` | Inject bundle data into the serialized content string |
| `PreDeserialize` | Extract and strip bundle data from content before parsing |
| `PostDeserialize` | Post-processing after the serializer parses content |

## Creating a Custom Asset Bundle Extender

### Step 1: Subclass USGDTAAssetBundleExtender

Create a new class that inherits from `USGDTAAssetBundleExtender`. The class must be concrete (not abstract) to be auto-discovered.

```cpp
// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Serialization/AssetBundleExtenders/SGDTAAssetBundleExtender.h"
#include "MyCustomAssetBundleExtender.generated.h"

/**
 * Custom asset bundle extender that stores bundles in a project-specific format.
 */
UCLASS(Blueprintable, BlueprintType)
class MYMODULE_API UMyCustomAssetBundleExtender : public USGDTAAssetBundleExtender
{
    GENERATED_BODY()

protected:
    // USGDTAAssetBundleExtender overrides
    virtual void Native_ExtractBundles(const UObject* Provider,
        FSGDynamicTextAssetBundleData& OutBundleData) const override;

    virtual void Native_PostSerialize(const FSGDynamicTextAssetBundleData& BundleData,
        FString& InOutContent, const FSGDTASerializerFormat& Format) const override;

    virtual bool Native_PreDeserialize(FString& InOutContent,
        FSGDynamicTextAssetBundleData& OutBundleData,
        const FSGDTASerializerFormat& Format) const override;
    // ~USGDTAAssetBundleExtender overrides
};
```

### Step 2: Override Native_ Methods

The most common overrides are `Native_PostSerialize` (to write bundle data into the file) and `Native_PreDeserialize` (to read bundle data back and strip it before the serializer parses the file).

`Native_ExtractBundles` typically does not need overriding unless you have a custom extraction strategy. The default implementation delegates to `ExtractFromObject()`.

```cpp
void UMyCustomAssetBundleExtender::Native_PostSerialize(
    const FSGDynamicTextAssetBundleData& BundleData,
    FString& InOutContent,
    const FSGDTASerializerFormat& Format) const
{
    if (BundleData.IsEmpty())
    {
        return;
    }

    // Inject bundle data into InOutContent in your custom format.
    // The Format parameter tells you which serializer format is being used
    // (JSON, XML, YAML) so you can adapt the output accordingly.
}

bool UMyCustomAssetBundleExtender::Native_PreDeserialize(
    FString& InOutContent,
    FSGDynamicTextAssetBundleData& OutBundleData,
    const FSGDTASerializerFormat& Format) const
{
    // Extract bundle data from InOutContent and populate OutBundleData.
    // Strip the bundle data from InOutContent so the serializer can parse
    // the remaining content without encountering unknown fields.
    // Return true if bundles were found and extracted, false otherwise.
    return false;
}
```

### Step 3: Automatic Registration

No manual registration is needed. On the next editor launch, `USGDynamicTextAssetRegistry` automatically discovers, assigns an `FSGDTAClassId`, and persists the manifest. See [Serializer Extenders](SerializerExtenders.md#automatic-discovery) for details on the discovery process.

### Hardcoding an Extender GUID (Advanced)

> **Warning:** Hardcoding GUIDs is not recommended. The automatic discovery system generates and persists stable IDs that remain consistent across sessions. Hardcoding bypasses this system and can cause conflicts if two extenders share the same GUID. Only use this approach if you have a specific requirement for a deterministic, cross-project GUID (e.g., a shared plugin where the extender must resolve to a known ID without a manifest file present).

If you need a deterministic GUID, override the manifest registration by pre-registering the extender with a known ID before automatic discovery runs. Add the following to your module's `StartupModule()`:

```cpp
void FMyModule::StartupModule()
{
    // Pre-register with a hardcoded GUID before automatic discovery runs.
    // Generate this GUID once (e.g., via FGuid::NewGuid() or an online generator)
    // and never change it.
    static const FSGDTAClassId MY_EXTENDER_ID = FSGDTAClassId::FromString(
        TEXT("A1B2C3D4-E5F6-7890-ABCD-EF1234567890"));

    USGDynamicTextAssetRegistry* registry = GEngine->GetEngineSubsystem<USGDynamicTextAssetRegistry>();
    if (registry)
    {
        TSharedPtr<FSGDTAExtenderManifest> manifest = registry->GetExtenderRegistry().GetManifest(
            SGDynamicTextAssetConstants::ASSET_BUNDLE_EXTENDER_FRAMEWORK_KEY);
        if (manifest.IsValid() && !manifest->FindByExtenderId(MY_EXTENDER_ID))
        {
            manifest->AddExtender(MY_EXTENDER_ID,
                TSoftClassPtr<UObject>(FSoftObjectPath(UMyCustomAssetBundleExtender::StaticClass())));
        }
    }
}
```

When automatic discovery runs later, it finds the class already in the manifest (matched by class name) and skips it, preserving your hardcoded ID.

## Settings Configuration

The settings data asset (`USGDynamicTextAssetSettingsAsset`) allows per-format extender overrides via `AssetBundleExtenderOverrides`. See [Settings](../Configuration/Settings.md) for the full settings reference.

```cpp
UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Asset Bundles")
TSet<FSGAssetBundleExtenderMapping> AssetBundleExtenderOverrides;
```

Each `FSGAssetBundleExtenderMapping` maps a serializer format bitmask (`FSGDTASerializerFormat AppliesTo`) to an extender class (`FSGDTAClassId ExtenderClassId`). Only one extender can be assigned per serializer format.

If no settings override is configured for a format, the system uses `USGDTADefaultAssetBundleExtender` as the baseline default.

## Per-DTA Override

Individual DTAs can override the asset bundle extender by setting the `AssetBundleExtenderOverride` property (an `FSGDTAClassId`) on the UObject. This takes the highest priority in the resolution order.

The override is serialized into the `sgFileInformation` section of the file under the key `assetBundleExtender`. When the override is invalid (default), the key is omitted.

## Resolution Order

When the serializer pipeline needs an asset bundle extender (via `USGDynamicTextAssetStatics::ResolveAssetBundleExtender`):

1. **Per-DTA override** - `FSGDTAClassId` set directly on the UObject
2. **Per-format settings override** - matched from `AssetBundleExtenderOverrides` by format bitmask
3. **Baseline default** - `USGDTADefaultAssetBundleExtender` looked up by class name via `FindExtenderIdByClass`

## Built-In Extenders

### USGDTADefaultAssetBundleExtender

The default extender. Stores bundles at the root level under a `sgdtAssetBundles` block, grouped by bundle name. Each entry contains a qualified property name and asset path.

**Header:** `SGDynamicTextAssetsRuntime/Public/Serialization/AssetBundleExtenders/SGDTADefaultAssetBundleExtender.h`

**Characteristics:**
- Fast lookup at load time (all bundles in one location)
- Standard format, good for most use cases
- Supports JSON, XML, and YAML via format-specific helper methods

**JSON format example:**

```json
{
  "sgFileInformation": { ... },
  "data": {
    "MeshRef": "/Game/Meshes/Sword",
    "TextureRef": "/Game/Textures/SwordTex"
  },
  "sgdtAssetBundles": {
    "Visual": [
      { "property": "UWeaponData.MeshRef", "path": "/Game/Meshes/Sword" },
      { "property": "UWeaponData.TextureRef", "path": "/Game/Textures/SwordTex" }
    ],
    "Audio": [
      { "property": "UWeaponData.SwingSound", "path": "/Game/Sounds/Swing" }
    ]
  }
}
```

### USGDTAPerPropertyAssetBundleExtender

Stores bundle metadata inline in the `data` section, next to each property's value. No root-level `sgdtAssetBundles` block is written.

**Header:** `SGDynamicTextAssetsRuntime/Public/Serialization/AssetBundleExtenders/SGDTAPerPropertyAssetBundleExtender.h`

**Characteristics:**
- Smaller file sizes (no duplicated asset paths)
- Slower bundle iteration at load time (data must be collected across the property tree)
- Supports JSON, XML, and YAML via format-specific helper methods

**JSON format example:**

```json
{
  "sgFileInformation": { ... },
  "data": {
    "MeshRef": {
      "value": "/Game/Meshes/Sword",
      "assetBundles": ["Visual"]
    },
    "TextureRef": {
      "value": "/Game/Textures/SwordTex",
      "assetBundles": ["Visual"]
    },
    "DamageAmount": 50.0
  }
}
```

Properties without asset bundles are stored in their normal format. Only soft reference properties with bundle assignments are wrapped in the `{ "value": ..., "assetBundles": [...] }` structure.

## Blueprint Extender Example

Extenders can also be created in Blueprint by subclassing `USGDTAAssetBundleExtender` and overriding the `BP_` layer methods:

1. Create a new Blueprint class with parent `USGDTAAssetBundleExtender`
2. Override the desired `BP_` events:
   - **Extract Bundles** - customize bundle extraction
   - **Pre Serialize** - pre-process content before writing
   - **Post Serialize** - inject bundle data after writing
   - **Pre Deserialize** - extract bundle data before parsing
   - **Post Deserialize** - post-process after parsing
3. The Blueprint class is auto-discovered on editor launch, same as C++ classes

The `BP_` layer is called by the `Notify` layer after the `Native_` layer, so Blueprint overrides supplement (not replace) C++ behavior. To fully replace behavior, subclass in C++ and override the `Native_` layer instead.

[Back to Table of Contents](../TableOfContents.md)
