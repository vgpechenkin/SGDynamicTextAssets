// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/SGDynamicTextAssetVersion.h"
#include "Core/SGDTASerializerFormat.h"

/**
 * Aggregated format version data for a single serializer type.
 * Tracks the distribution of file format versions across all DTA files
 * that use this serializer.
 */
struct SGDYNAMICTEXTASSETSEDITOR_API FSGDTASerializerFormatVersionInfo
{
	/** The serializer format (e.g., 1 for JSON, 2 for XML, 3 for YAML). */
	FSGDTASerializerFormat SerializerFormat;

	/** Human-readable serializer name (e.g., "JSON"). */
	FString SerializerName;

	/** File extension handled by this serializer (e.g., ".dta.json"). */
	FString FileExtension;

	/** The serializer's current file format version (from GetFileFormatVersion()). */
	FSGDynamicTextAssetVersion CurrentSerializerVersion;

	/** Highest file format version found across all files for this serializer. */
	FSGDynamicTextAssetVersion HighestFound;

	/** Lowest file format version found across all files for this serializer. */
	FSGDynamicTextAssetVersion LowestFound;

	/** Total number of DTA files using this serializer. */
	int32 TotalFileCount = 0;

	/** Count of files at each format version string (e.g., "1.0.0" -> 42). */
	TMap<FString, int32> CountByVersion;
};

/**
 * Project-level coordination manifest for the DTA plugin.
 *
 * Stored at Content/_SGDynamicTextAssets/ProjectManifest.json. Shared via source control.
 * Editor/development-only, NOT cooked or included in packaged builds.
 *
 * Responsibilities:
 * - Tracks the manifest folder organization version (for automated migration).
 * - Lists all type manifest and extender manifest file paths.
 * - Stores per-serializer format version distribution (absorbed from ProjectInfoCache).
 */
class SGDYNAMICTEXTASSETSEDITOR_API FSGDTAProjectManifest
{
public:

	static constexpr int32 CURRENT_ORGANIZATION_VERSION = 1;

	FSGDTAProjectManifest() = default;

	/** Save the manifest to a JSON file at the given path. */
	bool SaveToFile(const FString& FilePath) const;

	/** Load the manifest from a JSON file at the given path. */
	bool LoadFromFile(const FString& FilePath);

	/** Returns the default manifest file path under Content/_SGDynamicTextAssets/. */
	static FString GetDefaultManifestPath();

	// Organization version

	/** Returns the organization version from the loaded file (0 if not loaded). */
	int32 GetOrganizationVersion() const;

	/** Set the organization version. */
	void SetOrganizationVersion(int32 InVersion);

	/** Returns true if the loaded organization version matches CURRENT_ORGANIZATION_VERSION. */
	bool IsCurrentVersion() const;

	/** Returns true if the manifest has been loaded from disk. */
	bool IsLoaded() const;

	// Manifest path tracking (relative to GetInternalFilesRootPath)

	/** Get the relative paths to all tracked type manifest files. */
	const TArray<FString>& GetTypeManifestPaths() const;

	/** Set the relative paths to all type manifest files. */
	void SetTypeManifestPaths(const TArray<FString>& InPaths);

	/** Get the relative paths to all tracked extender manifest files. */
	const TArray<FString>& GetExtenderManifestPaths() const;

	/** Set the relative paths to all extender manifest files. */
	void SetExtenderManifestPaths(const TArray<FString>& InPaths);

	// Path resolution utilities

	/**
	 * Resolve a relative path against the internal files root.
	 * @param RelativePath Path relative to Content/_SGDynamicTextAssets/
	 * @return Absolute filesystem path
	 */
	static FString ResolveRelativePath(const FString& RelativePath);

	/**
	 * Convert an absolute path to a relative path from the internal files root.
	 * @param AbsolutePath Absolute filesystem path
	 * @return Path relative to Content/_SGDynamicTextAssets/
	 */
	static FString MakeRelativePath(const FString& AbsolutePath);

	// Format version tracking (absorbed from FSGDynamicTextAssetProjectInfoCache)

	/**
	 * Record a file's format version contribution.
	 * Called during the scan phase for each DTA file, and incrementally on file save.
	 *
	 * @param SerializerFormat The serializer format that handles this file
	 * @param FileFormatVersion The format version found in the file
	 */
	void RecordFileVersion(const FSGDTASerializerFormat& SerializerFormat, const FSGDynamicTextAssetVersion& FileFormatVersion);

	/**
	 * Populate the CurrentSerializerVersion field for all tracked serializers
	 * by querying the registered serializer instances.
	 * Call after scan phase completes.
	 */
	void PopulateCurrentSerializerVersions();

	/** Get format info for a specific serializer format. Returns nullptr if not tracked. */
	const FSGDTASerializerFormatVersionInfo* GetInfoForSerializer(const FSGDTASerializerFormat& SerializerFormat) const;

	/** Get all serializer format version infos. */
	const TMap<FSGDTASerializerFormat, FSGDTASerializerFormatVersionInfo>& GetAllFormatVersions() const;

	/** Reset all format version data. Call before a full rescan. */
	void ResetFormatVersionData();

	/** Mark the format version data as needing a rescan (e.g., after a file delete). */
	void MarkFormatVersionsDirty();

	/** Returns true if a full format version scan is needed (dirty or not loaded). */
	bool FormatVersionsNeedScan() const;

private:

	/** Organization version from the file. 0 means pre-v1 or not loaded. */
	int32 OrganizationVersion = 0;

	/** Relative paths to type manifest files within Content/_SGDynamicTextAssets/. */
	TArray<FString> TypeManifestRelativePaths;

	/** Relative paths to extender manifest files within Content/_SGDynamicTextAssets/. */
	TArray<FString> ExtenderManifestRelativePaths;

	/** Per-serializer format version tracking. Key = SerializerFormat. */
	TMap<FSGDTASerializerFormat, FSGDTASerializerFormatVersionInfo> FormatVersionsBySerializerId;

	/** True if the manifest has been loaded from disk. */
	uint8 bIsLoaded : 1 = false;

	/** True if the format version data is stale and needs a rescan. */
	uint8 bFormatVersionsDirty : 1 = true;
};
