// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/SGDTAClassId.h"
#include "UObject/SoftObjectPtr.h"

class FSGDTAExtenderManifest;

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
 * Manifest manager that coordinates multiple FSGDTAExtenderManifest instances,
 * one per extender framework.
 *
 * All per-entry operations (add, remove, find, lookup) go through individual
 * manifests obtained via GetOrCreateManifest() or GetManifest(). The registry
 * handles directory-level persistence: scanning for manifest files, loading
 * them, and saving dirty manifests back to disk.
 *
 * File naming convention: DTA_{FrameworkKey}_extenders.dta.json / .dta.bin
 * Example: DTA_AssetBundleExtenders_extenders.dta.json
 */
class SGDYNAMICTEXTASSETSRUNTIME_API FSGDTASerializerExtenderRegistry
{
public:

	FSGDTASerializerExtenderRegistry() = default;

	/**
	 * Returns the manifest for the given framework key, creating one if it does not exist.
	 * The returned pointer is never null.
	 *
	 * @param FrameworkKey Identifies which extender framework this manifest serves
	 * @return Shared pointer to the manifest (always valid)
	 */
	TSharedPtr<FSGDTAExtenderManifest> GetOrCreateManifest(FName FrameworkKey);

	/**
	 * Returns the manifest for the given framework key, or nullptr if none exists.
	 *
	 * @param FrameworkKey Identifies which extender framework to look up
	 * @return Shared pointer to the manifest, or nullptr if not found
	 */
	TSharedPtr<FSGDTAExtenderManifest> GetManifest(FName FrameworkKey) const;

	/**
	 * Returns all registered framework keys.
	 *
	 * @return Array of framework keys for all managed manifests
	 */
	TArray<FName> GetAllManifestKeys() const;

	/**
	 * Scans a directory for manifest files and loads each into a managed manifest.
	 * Files must match the naming pattern: DTA_{FrameworkKey}_extenders.dta.json
	 *
	 * @param Directory Absolute path to the directory containing manifest files
	 * @return True if at least one manifest was loaded successfully
	 */
	bool LoadAllManifests(const FString& Directory);

	/**
	 * Saves all dirty manifests to the given directory.
	 * Each manifest is written as DTA_{FrameworkKey}_extenders.dta.json.
	 *
	 * @param Directory Absolute path to write manifest files
	 * @return True if all saves succeeded
	 */
	bool SaveAllManifests(const FString& Directory) const;

	/**
	 * Scans a directory for binary manifest files and loads each into a managed manifest.
	 * Files must match the naming pattern: DTA_{FrameworkKey}_extenders.dta.bin
	 *
	 * @param Directory Absolute path to the directory containing binary manifest files
	 * @return True if at least one manifest was loaded successfully
	 */
	bool LoadAllManifestsBinary(const FString& Directory);

	/**
	 * Saves all managed manifests as binary files to the given directory.
	 * Each manifest is written as DTA_{FrameworkKey}_extenders.dta.bin.
	 * Unlike SaveAllManifests, this always writes every manifest regardless of dirty state.
	 *
	 * @param CookedDirectory Absolute path to write binary manifest files
	 * @return True if all saves succeeded
	 */
	bool BakeAllManifests(const FString& CookedDirectory) const;

	/** Returns true if any managed manifest has been modified since last save/load. */
	bool HasAnyDirty() const;

	/** Removes all managed manifests. */
	void ClearAllManifests();

	/** Returns the number of managed manifests. */
	int32 NumManifests() const;

	/** Returns true if managed manifests is empty. False if there are actual manifests. */
	bool IsEmptyManfiests() const;

	/**
	 * Searches all managed manifests for an entry matching the given class.
	 * Returns the extender ID if found, or FSGDTAClassId::INVALID_CLASS_ID.
	 *
	 * @param InClass The UClass to look up
	 * @return The extender's ClassId, or INVALID_CLASS_ID if not registered
	 */
	FSGDTAClassId FindExtenderIdByClass(const UClass* InClass) const;

private:

	/** All managed manifests, keyed by framework name. */
	TMap<FName, TSharedRef<FSGDTAExtenderManifest>> ManagedManifests;
};
