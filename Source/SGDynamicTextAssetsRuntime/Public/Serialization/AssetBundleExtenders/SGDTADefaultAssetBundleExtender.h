// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Serialization/AssetBundleExtenders/SGDTAAssetBundleExtender.h"

#include "SGDTADefaultAssetBundleExtender.generated.h"

/**
 * Default asset bundle extender that implements the root-level bundle block approach.
 *
 * Supports all natively implemented serializer formats (JSON, XML, YAML).
 * Dispatches to format-specific helpers based on the FSGDTASerializerFormat parameter.
 *
 * Writes bundles as a top-level "sgdtAssetBundles" block grouped by bundle name.
 * Each bundle contains an array of entries with "property" and "path" fields.
 *
 * This is the standard format and matches the original hardcoded behavior
 * from the serializers. Fast bundle lookup at load time since all
 * bundles are in a single block.
 */
UCLASS(Blueprintable, BlueprintType, ClassGroup = "Start Games", DisplayName = "SGDTA Default Asset Bundle Extender")
class SGDYNAMICTEXTASSETSRUNTIME_API USGDTADefaultAssetBundleExtender : public USGDTAAssetBundleExtender
{
	GENERATED_BODY()

protected:

	// USGDTAAssetBundleExtender overrides
	virtual void Native_SerializeBundles(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutSerializedContent, const FSGDTASerializerFormat& Format) const override;
	virtual bool Native_DeserializeBundles(const FString& SerializedContent,
		FSGDynamicTextAssetBundleData& OutBundleData, const FSGDTASerializerFormat& Format) const override;
	// ~USGDTAAssetBundleExtender overrides

	// Per-format serialize helpers
	void SerializeBundlesJson(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutContent) const;
	void SerializeBundlesXml(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutContent) const;
	void SerializeBundlesYaml(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutContent) const;

	// Per-format deserialize helpers
	bool DeserializeBundlesJson(const FString& Content,
		FSGDynamicTextAssetBundleData& OutBundleData) const;
	bool DeserializeBundlesXml(const FString& Content,
		FSGDynamicTextAssetBundleData& OutBundleData) const;
	bool DeserializeBundlesYaml(const FString& Content,
		FSGDynamicTextAssetBundleData& OutBundleData) const;
};
