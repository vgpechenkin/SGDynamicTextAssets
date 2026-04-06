# Serializer Extenders

[Back to Table of Contents](../TableOfContents.md)

## Overview

The Serializer Extender Framework provides a modular extension system for customizing how serializers handle specific aspects of DTA file serialization. Extenders are UObject-based classes that can be subclassed in C++ or Blueprint to override default behavior without modifying the core serializer classes.

**When to use extenders:**
- You need a different storage strategy for a serialization concern (e.g., asset bundle storage)
- You want to add custom pre/post-processing to the serialization pipeline
- You want project-specific serialization behavior without forking the core serializers

**Technical design reference:** See Section 18 of `SGDynamicTextAssets_Technical_Design.md` for the full architectural design.

## Architecture

The framework consists of three layers:

1. **Base class** (`USGDTASerializerExtenderBase`) - abstract UObject root for all extenders
2. **Concrete extender types** (e.g., `USGDTAAssetBundleExtender`) - define the API for a specific serialization concern
3. **Registry and manifests** (`FSGDTASerializerExtenderRegistry`, `FSGDTAExtenderManifest`) - manage identity, persistence, and resolution

Each extender class is identified by an `FSGDTAClassId` (a GUID wrapper) assigned during automatic discovery and persisted in manifest files. This decouples class identity from C++ class names.

## Class Hierarchy

```
USGDTASerializerExtenderBase              (Abstract base, Runtime module)
    └── USGDTAAssetBundleExtender         (Asset bundle strategy, Runtime module)
            ├── USGDTADefaultAssetBundleExtender       (Root-level bundles)
            └── USGDTAPerPropertyAssetBundleExtender   (Inline per-property bundles)
```

Future extender types (beyond asset bundles) would subclass `USGDTASerializerExtenderBase` directly and define their own API contract.

## FSGDTAClassId

A reusable USTRUCT wrapping `FGuid` for identifying extender classes in manifests and registries. See Section 18.2 of the technical design document for the full API.

**Header:** `SGDynamicTextAssetsRuntime/Public/Core/SGDTAClassId.h`

Key properties:
- Stable across sessions (persisted in manifest files)
- Blueprint-exposed with a custom editor class picker
- Supports `meta=(SGDTAClassType="USGDTAAssetBundleExtender")` for filtering to a base class

## Registry and Manifests

`FSGDTASerializerExtenderRegistry` is a manifest manager that coordinates multiple per-framework manifest files. Each extender framework gets its own `FSGDTAExtenderManifest` instance identified by a framework key (`FName`).

**File naming:** `DTA_{FrameworkKey}_Extenders.dta.json` (editor) / `DTA_{FrameworkKey}_Extenders.dta.bin` (cooked)

**Key API:**
- `GetOrCreateManifest(FName FrameworkKey)` - returns a manifest for the specified framework
- `GetManifest(FName FrameworkKey)` - returns existing manifest or nullptr
- `LoadAllManifests(Directory)` / `SaveAllManifests(Directory)` - JSON persistence
- `LoadAllManifestsBinary(Directory)` / `BakeAllManifests(CookedDirectory)` - binary persistence for cooked builds
- `FindExtenderIdByClass(const UClass*)` - cross-manifest lookup by class name

Each `FSGDTAExtenderManifest` stores entries mapping `FSGDTAClassId` to class info (soft class path, class name) and supports server overlay for runtime overrides without modifying local files.

## Automatic Discovery

Extender classes are discovered via reflection rather than manual registration. On editor startup, `USGDynamicTextAssetRegistry::LoadAndPersistExtenderManifests()`:

1. Loads existing manifests from `{InternalFilesRoot}/SerializerExtenders/`
2. Scans for all concrete subclasses via `GetDerivedClasses()`
3. Assigns a new `FSGDTAClassId` to any class not already in the manifest
4. Updates stale class paths for existing entries
5. Saves dirty manifests to disk

No hardcoded GUIDs exist. All extender IDs are generated on first discovery and persist in the manifest file.

## Extender Types

The framework currently supports one extender type:

- [Asset Bundle Extenders](AssetBundleExtenders.md) - controls how asset bundle metadata is extracted, serialized, and deserialized

[Back to Table of Contents](../TableOfContents.md)
