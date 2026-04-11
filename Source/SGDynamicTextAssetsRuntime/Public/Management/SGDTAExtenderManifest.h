// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Management/SGDTASerializerExtenderRegistry.h"

class FJsonObject;

/**
 * Single manifest file for one extender framework.
 *
 * Stores extender entries (ID, class, class name), provides CRUD and lookup,
 * handles JSON persistence, and supports runtime server overlay. Each extender
 * framework (asset bundle extenders, future systems) gets its own manifest
 * instance managed by the registry.
 *
 * Manifest JSON format:
 * {
 *   "schema": "dta_extender_manifest",
 *   "version": 1,
 *   "framework": "AssetBundles",
 *   "extenders": [
 *     {
 *       "extenderId": "A1B2C3D4-...",
 *       "className": "SGDTADefaultAssetBundleExtender",
 *       "classPath": "/Script/SGDynamicTextAssetsRuntime.SGDTADefaultAssetBundleExtender"
 *     }
 *   ]
 * }
 *
 * Lookups use linear scans on the entries array. This is intentional:
 * manifests contain very few entries (<20), making linear scans faster
 * than hash-based lookups due to cache locality and zero hashing overhead.
 *
 * Server overlay support: server-provided extender data can override or
 * extend the local manifest without modifying the on-disk file. Overlay
 * entries take precedence over local entries when resolving via
 * GetEffectiveEntry().
 */
class SGDYNAMICTEXTASSETSRUNTIME_API FSGDTAExtenderManifest
{
public:

	FSGDTAExtenderManifest();
	explicit FSGDTAExtenderManifest(FName InFrameworkKey);

	/**
	 * Loads the manifest from a JSON file.
	 *
	 * @param FilePath Absolute path to the manifest JSON file
	 * @return True if file was loaded and parsed successfully
	 */
	bool LoadFromFile(const FString& FilePath);

	/**
	 * Saves the manifest to a JSON file.
	 *
	 * @param FilePath Absolute path to write the manifest JSON file
	 * @return True if file was written successfully
	 */
	bool SaveToFile(const FString& FilePath) const;

	/**
	 * Saves the manifest to a compact binary file for cooked builds.
	 * Uses string table deduplication for efficient storage.
	 *
	 * @param FilePath Absolute path to write the binary file
	 * @return True if file was written successfully
	 */
	bool SaveToBinaryFile(const FString& FilePath) const;

	/**
	 * Loads the manifest from a binary file (cooked build format).
	 *
	 * @param FilePath Absolute path to the binary manifest file
	 * @return True if file was loaded and parsed successfully
	 */
	bool LoadFromBinaryFile(const FString& FilePath);

	/**
	 * Finds a local manifest entry by extender ID.
	 * Does not consider server overlays. Use GetEffectiveEntry() for that.
	 *
	 * @param ExtenderId The extender ID to search for
	 * @return Pointer to the entry if found, nullptr otherwise
	 */
	const FSGDTASerializerExtenderRegistryEntry* FindByExtenderId(const FSGDTAClassId& ExtenderId) const;

	/**
	 * Finds a local manifest entry by class name.
	 * Does not consider server overlays.
	 *
	 * @param ClassName Class name to search for
	 * @return Pointer to the entry if found, nullptr otherwise
	 */
	const FSGDTASerializerExtenderRegistryEntry* FindByClassName(const FString& ClassName) const;

	/**
	 * Adds an extender entry to the manifest.
	 * If an extender with the same ID already exists, it is replaced.
	 *
	 * @param ExtenderId Unique extender identifier
	 * @param InClass Soft class reference to the extender class
	 */
	void AddExtender(const FSGDTAClassId& ExtenderId, const TSoftClassPtr<UObject>& InClass);

	/**
	 * Removes an extender entry from the manifest.
	 *
	 * @param ExtenderId The extender ID to remove
	 * @return True if the entry was found and removed
	 */
	bool RemoveExtender(const FSGDTAClassId& ExtenderId);

	/** Returns all local extender entries. Does not include server overlay entries. */
	const TArray<FSGDTASerializerExtenderRegistryEntry>& GetAllExtenders() const;

	/**
	 * Returns all effective extender entries, combining local entries with server overlay.
	 * Local entries overridden by server are replaced, server-disabled entries are excluded,
	 * and server-added entries (not present locally) are appended.
	 *
	 * @param OutEntries Array to populate with all effective extender entries
	 */
	void GetAllEffectiveExtenders(TArray<FSGDTASerializerExtenderRegistryEntry>& OutEntries) const;

	/**
	 * Returns the soft class pointer for a given extender ID.
	 * Checks the server overlay first, then local entries.
	 *
	 * @param ExtenderId The extender ID to look up
	 * @return The soft class pointer, or a null TSoftClassPtr if not found
	 */
	TSoftClassPtr<UObject> GetSoftClassPtr(const FSGDTAClassId& ExtenderId) const;

	/**
	 * Returns the soft class pointer for a given class name.
	 * Only checks local entries (not server overlay).
	 *
	 * @param ClassName Class name to look up
	 * @return The soft class pointer, or a null TSoftClassPtr if not found
	 */
	TSoftClassPtr<UObject> GetSoftClassPtrByClassName(const FString& ClassName) const;

	/**
	 * Returns the effective entry for an extender ID, considering server overlay.
	 * Server overlay entries take precedence over local entries.
	 * An overlay entry with empty ClassName means the extender is disabled.
	 *
	 * @param ExtenderId The extender ID to look up
	 * @return Pointer to the effective entry, or nullptr if not found or disabled
	 */
	const FSGDTASerializerExtenderRegistryEntry* GetEffectiveEntry(const FSGDTAClassId& ExtenderId) const;

	/**
	 * Applies server-provided extender overrides.
	 * Server overlay is stored separately; the local manifest file is never modified.
	 *
	 * Expected JSON format:
	 * {
	 *   "extenders": [
	 *     { "extenderId": "...", "className": "..." }
	 *   ]
	 * }
	 *
	 * Server can: add new extender entries, override existing entries,
	 * or disable entries by providing an entry with an empty className.
	 *
	 * @param ServerData JSON object containing server extender overrides
	 */
	void ApplyServerOverrides(const TSharedPtr<FJsonObject>& ServerData);

	/** Clears all server overlay entries, reverting to local-only state. */
	void ClearServerOverrides();

	/** Returns true if there are any active server overlay entries. */
	bool HasServerOverrides() const;

	/** Returns the number of local extender entries. */
	int32 Num() const;

	/** Returns true if the manifest has been modified since last save/load. */
	bool IsDirty() const;

	/** Clears all local entries. Does not affect server overlay. */
	void Clear();

	/** Returns the framework key identifying which extender system this manifest serves. */
	FName GetFrameworkKey() const;

	/** Current manifest format version. */
	static constexpr int32 MANIFEST_VERSION = 1;

	/** Key for schema type. */
	static const FString KEY_SCHEMA;

	/** Value for schema type. */
	static const FString VALUE_SCHEMA;

	/** Key for manifest version. */
	static const FString KEY_VERSION;

	/** Key for the framework identifier. */
	static const FString KEY_FRAMEWORK;

	/** Key for extenders array. */
	static const FString KEY_EXTENDERS;

	/** Key for an extender entry's extender ID. */
	static const FString KEY_EXTENDER_ID;

	/** Key for an extender entry's class name. */
	static const FString KEY_CLASS_NAME;

	/** Key for an extender entry's full soft class path. */
	static const FString KEY_CLASS_PATH;

private:

	/**
	 * Parses a single extender entry from a JSON object.
	 *
	 * @param EntryObject JSON object to parse
	 * @param OutEntry Populated on success
	 * @return True if the entry was valid and parsed successfully
	 */
	static bool ParseExtenderEntry(const TSharedPtr<FJsonObject>& EntryObject,
		FSGDTASerializerExtenderRegistryEntry& OutEntry);

	/** Identifies which extender framework this manifest belongs to. */
	FName FrameworkKey;

	/** All local manifest entries. */
	TArray<FSGDTASerializerExtenderRegistryEntry> Entries;

	/** Server overlay entries. Stored separately from local entries. */
	TMap<FSGDTAClassId, FSGDTASerializerExtenderRegistryEntry> ServerOverlayEntries;

	/** Whether the manifest has been modified since last save/load. */
	uint8 bIsDirty : 1 = 0;
};
