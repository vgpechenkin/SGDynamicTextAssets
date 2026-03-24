// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/SGDTAClassId.h"
#include "UObject/SoftObjectPtr.h"

class FJsonObject;

/**
 * Single entry in the serializer extender registry.
 * Maps an extender ID to a class for lookup and resolution.
 */
struct SGDYNAMICTEXTASSETSRUNTIME_API FSGDTASerializerExtenderRegistryEntry
{
public:

	FSGDTASerializerExtenderRegistryEntry() = default;

	FSGDTASerializerExtenderRegistryEntry(const FSGDTAClassId& InExtenderId,
		const TSoftClassPtr<UObject>& InClass);

	/** Unique identifier for this extender class. */
	FSGDTAClassId ExtenderId;

	/** Soft reference to the extender UClass. Avoids hard load dependencies. */
	TSoftClassPtr<UObject> Class = nullptr;

	/**
	 * Cached class name extracted from the soft class path.
	 * Populated during construction and LoadFromFile() for fast lookup
	 * without resolving the soft pointer.
	 */
	FString ClassName;
};

/**
 * Flat registry mapping FSGDTAClassId to serializer extender UClasses.
 *
 * Follows the FSGDynamicTextAssetTypeManifest pattern but without
 * hierarchy (no parent IDs, no root type ID). Provides O(1) lookup
 * by extender ID or class name, JSON persistence with schema versioning,
 * and runtime server overlay support.
 *
 * Registry JSON format:
 * {
 *   "schema": "dta_extender_registry",
 *   "version": 1,
 *   "extenders": [
 *     {
 *       "extenderId": "A1B2C3D4-...",
 *       "className": "USGDefaultAssetBundleExtender",
 *       "classPath": "/Script/SGDynamicTextAssetsRuntime.USGDefaultAssetBundleExtender"
 *     }
 *   ]
 * }
 *
 * Server overlay support: server-provided extender data can override or
 * extend the local registry without modifying the on-disk file. Overlay
 * entries take precedence over local entries when resolving via
 * GetEffectiveEntry().
 */
class SGDYNAMICTEXTASSETSRUNTIME_API FSGDTASerializerExtenderRegistry
{
public:

	FSGDTASerializerExtenderRegistry() = default;

	/**
	 * Loads the registry from a JSON file.
	 *
	 * @param FilePath Absolute path to the registry JSON file
	 * @return True if file was loaded and parsed successfully
	 */
	bool LoadFromFile(const FString& FilePath);

	/**
	 * Saves the registry to a JSON file.
	 *
	 * @param FilePath Absolute path to write the registry JSON file
	 * @return True if file was written successfully
	 */
	bool SaveToFile(const FString& FilePath) const;

	/**
	 * Finds a local registry entry by extender ID. O(1) lookup.
	 * Does not consider server overlays. Use GetEffectiveEntry() for that.
	 *
	 * @param ExtenderId The extender ID to search for
	 * @return Pointer to the entry if found, nullptr otherwise
	 */
	const FSGDTASerializerExtenderRegistryEntry* FindByExtenderId(const FSGDTAClassId& ExtenderId) const;

	/**
	 * Finds a local registry entry by class name. O(1) lookup.
	 * Does not consider server overlays.
	 *
	 * @param ClassName Class name to search for (e.g., "USGDefaultAssetBundleExtender")
	 * @return Pointer to the entry if found, nullptr otherwise
	 */
	const FSGDTASerializerExtenderRegistryEntry* FindByClassName(const FString& ClassName) const;

	/**
	 * Adds an extender entry to the registry.
	 * If an extender with the same ID already exists, it is replaced.
	 *
	 * @param ExtenderId Unique extender identifier
	 * @param InClass Soft class reference to the extender class
	 */
	void AddExtender(const FSGDTAClassId& ExtenderId, const TSoftClassPtr<UObject>& InClass);

	/**
	 * Removes an extender entry from the registry.
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

	/** Returns the number of local extender entries. */
	int32 Num() const;

	/** Returns true if the registry has been modified since last save/load. */
	bool IsDirty() const;

	/** Clears all local entries and indices. Does not affect server overlay. */
	void Clear();

	/**
	 * Applies server-provided extender overrides.
	 * Server overlay is stored separately; the local registry file is never modified.
	 *
	 * Expected JSON format (same "extenders" array as the registry):
	 * {
	 *   "extenders": [
	 *     { "extenderId": "...", "className": "..." }
	 *   ]
	 * }
	 *
	 * Server can: add new extender entries, override existing entries (remap ID
	 * to different class), or remove entries by providing an entry with an empty
	 * className.
	 *
	 * @param ServerData JSON object containing server extender overrides
	 */
	void ApplyServerOverrides(const TSharedPtr<FJsonObject>& ServerData);

	/**
	 * Returns the effective entry for an extender ID, considering server overlay.
	 * Server overlay entries take precedence over local entries.
	 * An overlay entry with empty ClassName means the extender is disabled.
	 *
	 * @param ExtenderId The extender ID to look up
	 * @return Pointer to the effective entry, or nullptr if not found or disabled
	 */
	const FSGDTASerializerExtenderRegistryEntry* GetEffectiveEntry(const FSGDTAClassId& ExtenderId) const;

	/** Clears all server overlay entries, reverting to local-only state. */
	void ClearServerOverrides();

	/** Returns true if there are any active server overlay entries. */
	bool HasServerOverrides() const;

	/** Current registry format version. */
	static constexpr int32 REGISTRY_VERSION = 1;

	/** Key for schema type. */
	static const FString KEY_SCHEMA;

	/** Value for schema type. */
	static const FString VALUE_SCHEMA;

	/** Key for registry version. */
	static const FString KEY_VERSION;

	/** Key for extenders array. */
	static const FString KEY_EXTENDERS;

	/** Key for an extender entry's extender ID. */
	static const FString KEY_EXTENDER_ID;

	/** Key for an extender entry's class name. */
	static const FString KEY_CLASS_NAME;

	/** Key for an extender entry's full soft class path. */
	static const FString KEY_CLASS_PATH;

private:

	/** Rebuilds the ExtenderId and ClassName lookup indices from the entries array. */
	void RebuildIndices();

	/**
	 * Parses a single extender entry from a JSON object.
	 *
	 * @param EntryObject JSON object to parse
	 * @param OutEntry Populated on success
	 * @return True if the entry was valid and parsed successfully
	 */
	static bool ParseExtenderEntry(const TSharedPtr<FJsonObject>& EntryObject,
		FSGDTASerializerExtenderRegistryEntry& OutEntry);

	/** All local registry entries. */
	TArray<FSGDTASerializerExtenderRegistryEntry> Extenders;

	/** ExtenderId -> index into Extenders array for O(1) lookup. */
	TMap<FSGDTAClassId, int32> ExtenderIdIndex;

	/** ClassName -> index into Extenders array for O(1) lookup. */
	TMap<FString, int32> ClassNameIndex;

	/** Server overlay entries. Stored separately from local entries. */
	TMap<FSGDTAClassId, FSGDTASerializerExtenderRegistryEntry> ServerOverlayEntries;

	/** Whether the registry has been modified since last save/load. */
	uint8 bIsDirty : 1 = 0;
};
