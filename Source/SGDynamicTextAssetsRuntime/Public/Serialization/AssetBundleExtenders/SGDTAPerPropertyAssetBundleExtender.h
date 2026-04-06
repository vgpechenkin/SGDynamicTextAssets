// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Dom/JsonObject.h"
#include "Serialization/AssetBundleExtenders/SGDTAAssetBundleExtender.h"

#include "SGDTAPerPropertyAssetBundleExtender.generated.h"

/**
 * Per-property asset bundle extender that stores bundle metadata inline
 * next to each property's value in the data section.
 *
 * Supports all natively implemented serializer formats (JSON, XML, YAML).
 * Dispatches to format-specific helpers based on the FSGDTASerializerFormat parameter.
 *
 * Instead of writing a root-level "sgdtAssetBundles" block, this extender
 * wraps each bundled property's value with an object containing the original
 * value and an "assetBundles" array listing the bundle names.
 *
 * Example JSON output:
 *   "data": {
 *     "MeshAsset": { "value": "/Game/Meshes/Sword", "assetBundles": ["Visual"] },
 *     "Damage": 50
 *   }
 *
 * Produces smaller file sizes than the default extender when assets have
 * many bundled references, but has slower bundle iteration at load time
 * since bundle data is scattered across the data section.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = "Start Games", DisplayName = "SGDTA Per-Property Asset Bundle Extender")
class SGDYNAMICTEXTASSETSRUNTIME_API USGDTAPerPropertyAssetBundleExtender : public USGDTAAssetBundleExtender
{
	GENERATED_BODY()

protected:

	// USGDTAAssetBundleExtender overrides
	virtual void Native_PostSerialize(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutContent, const FSGDTASerializerFormat& Format) const override;
	virtual bool Native_PreDeserialize(FString& InOutContent,
		FSGDynamicTextAssetBundleData& OutBundleData, const FSGDTASerializerFormat& Format) const override;
	// ~USGDTAAssetBundleExtender overrides

	// Per-format serialize helpers
	void SerializeBundlesJson(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutContent) const;
	void SerializeBundlesXml(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutContent) const;
	void SerializeBundlesYaml(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutContent) const;

	// Per-format deserialize helpers (extract bundles AND unwrap properties in-place)
	bool DeserializeBundlesJson(FString& InOutContent,
		FSGDynamicTextAssetBundleData& OutBundleData) const;
	bool DeserializeBundlesXml(FString& InOutContent,
		FSGDynamicTextAssetBundleData& OutBundleData) const;
	bool DeserializeBundlesYaml(FString& InOutContent,
		FSGDynamicTextAssetBundleData& OutBundleData) const;

public:

	/** Two-level map: ClassName -> (PropertyName -> BundleNames). */
	using FClassPropertyBundlesMap = TMap<FString, TMap<FString, TArray<FString>>>;

	/**
	 * Extracts the short class name from a type marker value.
	 * For SG_INST_OBJ_CLASS paths like "/Game/BP_Test.BP_Test_C", returns "BP_Test_C".
	 * For SG_STRUCT_TYPE / SG_INST_STRUCT_TYPE values, returns the value as-is.
	 *
	 * @param MarkerValue The raw marker value string
	 * @param bIsClassPath True if the value is a full class path (SG_INST_OBJ_CLASS)
	 * @return The short class/struct name for use as a bundle map key
	 */
	static FString ExtractClassNameFromMarker(const FString& MarkerValue, bool bIsClassPath);

	/**
	 * Finds or creates a bundle in the output bundle data by name.
	 *
	 * @param OutBundleData The bundle data to search/modify
	 * @param BundleName The bundle name to find or create
	 * @return Pointer to the found or created bundle
	 */
	static FSGDynamicTextAssetBundle* FindOrCreateBundle(
		FSGDynamicTextAssetBundleData& OutBundleData, FName BundleName);

private:

	/**
	 * Builds a two-level lookup preserving class qualifiers from property names.
	 * Qualified names like "UWeaponData.MeshAsset" split into ClassName="UWeaponData",
	 * PropertyName="MeshAsset". Unqualified names use an empty string as the class key.
	 *
	 * @param BundleData The bundle data containing qualified property names
	 * @return Two-level map of ClassName -> PropertyName -> BundleNames
	 */
	static FClassPropertyBundlesMap BuildPropertyToBundlesMap(
		const FSGDynamicTextAssetBundleData& BundleData);

	/**
	 * Determines the root class name by matching class entries in the bundle map
	 * against top-level fields in the data JSON object.
	 *
	 * @param DataObject The top-level data JSON object
	 * @param ClassPropertyBundles The two-level bundle map
	 * @return The class name whose properties best match the top-level data fields
	 */
	static FString DetermineRootClassName(
		const TSharedPtr<FJsonObject>& DataObject,
		const FClassPropertyBundlesMap& ClassPropertyBundles);

	/**
	 * Recursively wraps bundled properties in a JSON object with inline bundle metadata.
	 * Detects SG_INST_OBJ_CLASS, SG_STRUCT_TYPE, and SG_INST_STRUCT_TYPE markers
	 * to recurse into nested objects with the correct class context.
	 *
	 * @param JsonObject The JSON object to process
	 * @param ClassName The class context for looking up bundle entries
	 * @param ClassPropertyBundles The two-level bundle map
	 */
	static void WrapBundledPropertiesRecursive(
		const TSharedPtr<FJsonObject>& JsonObject,
		const FString& ClassName,
		const FClassPropertyBundlesMap& ClassPropertyBundles);

	/**
	 * Recursively extracts bundle data from a JSON object containing inline bundle metadata.
	 * Detects wrapped properties (objects with "value" + "assetBundles") and type markers
	 * to recurse into nested objects.
	 *
	 * @param JsonObject The JSON object to scan
	 * @param ClassName The class context for qualifying extracted property names
	 * @param OutBundleData The bundle data to populate
	 */
	static void ExtractBundlesFromJsonObjectRecursive(
		const TSharedPtr<FJsonObject>& JsonObject,
		const FString& ClassName,
		FSGDynamicTextAssetBundleData& OutBundleData);

};
