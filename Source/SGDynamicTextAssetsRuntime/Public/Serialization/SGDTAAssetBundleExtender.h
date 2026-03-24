// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/SGDynamicTextAssetBundleData.h"
#include "Serialization/SGDTASerializerExtenderBase.h"

#include "SGDTAAssetBundleExtender.generated.h"

/**
 * Abstract base class for asset bundle extenders.
 *
 * Defines the strategy for how asset bundle metadata is extracted from DTA
 * objects, serialized into files, and deserialized back. Different extenders
 * can produce different file layouts (e.g., root-level bundle block vs
 * per-property inline metadata).
 *
 * Uses a three-layer API:
 *
 * Layer 1 - Notify (public, non-virtual):
 *   Called by the serializer pipeline. Validates inputs, then calls Native_
 *   followed by BP_. Orchestrates the full flow.
 *
 * Layer 2 - Native_ (virtual, C++ only):
 *   Core C++ behavior. C++ subclasses override this for full control.
 *   Does not call BP_ - that is Notify's responsibility.
 *
 * Layer 3 - BP_ (BlueprintNativeEvent):
 *   Blueprint-overridable hook. Called by Notify after Native_.
 *   Blueprint subclasses customize behavior here without touching
 *   the C++ logic.
 *
 * Serialize/Deserialize methods use FString& instead of FJsonObject to
 * be format-agnostic. The extender receives the full serialized text and
 * handles parsing internally for whatever format it expects.
 *
 * Operates on a single instance on this machine.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = "Start Games")
class SGDYNAMICTEXTASSETSRUNTIME_API USGDTAAssetBundleExtender : public USGDTASerializerExtenderBase
{
	GENERATED_BODY()
public:

	USGDTAAssetBundleExtender() = default;

	/**
	 * Extracts asset bundle data from a provider object.
	 * Validates the provider, then calls Native_ExtractBundles
	 * followed by BP_ExtractBundles.
	 *
	 * @param Provider The UObject to extract bundle data from
	 * @param OutBundleData Populated with extracted bundle metadata
	 */
	void NotifyExtractBundles(const UObject* Provider,
		FSGDynamicTextAssetBundleData& OutBundleData) const;

	/**
	 * Writes bundle data into the serialized content string.
	 * Calls Native_SerializeBundles followed by BP_SerializeBundles.
	 *
	 * @param BundleData The bundle data to serialize
	 * @param InOutSerializedContent The full serialized file content to modify
	 */
	void NotifySerializeBundles(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutSerializedContent) const;

	/**
	 * Reads bundle data from the serialized content string.
	 * Calls Native_DeserializeBundles followed by BP_DeserializeBundles.
	 *
	 * @param SerializedContent The full serialized file content to read from
	 * @param OutBundleData Populated with deserialized bundle metadata
	 * @return True if bundle data was successfully deserialized
	 */
	bool NotifyDeserializeBundles(const FString& SerializedContent,
		FSGDynamicTextAssetBundleData& OutBundleData) const;

protected:

	/**
	 * C++ implementation of bundle extraction.
	 * Default: calls OutBundleData.ExtractFromObject(Provider) in editor builds.
	 *
	 * @param Provider The UObject to extract bundle data from
	 * @param OutBundleData Populated with extracted bundle metadata
	 */
	virtual void Native_ExtractBundles(const UObject* Provider,
		FSGDynamicTextAssetBundleData& OutBundleData) const;

	/**
	 * C++ implementation of bundle serialization.
	 * Default: no-op. Subclasses must override to produce meaningful output.
	 *
	 * @param BundleData The bundle data to serialize
	 * @param InOutSerializedContent The full serialized file content to modify
	 */
	virtual void Native_SerializeBundles(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutSerializedContent) const;

	/**
	 * C++ implementation of bundle deserialization.
	 * Default: returns true (passthrough).
	 *
	 * @param SerializedContent The full serialized file content to read from
	 * @param OutBundleData Populated with deserialized bundle metadata
	 * @return True if bundle data was successfully deserialized
	 */
	virtual bool Native_DeserializeBundles(const FString& SerializedContent,
		FSGDynamicTextAssetBundleData& OutBundleData) const;

	/**
	 * Blueprint hook for additional bundle extraction logic.
	 * Called by NotifyExtractBundles after Native_ExtractBundles.
	 * Default: no-op.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Asset Bundles",
		meta = (DisplayName = "Extract Bundles"))
	void BP_ExtractBundles(const UObject* Provider,
		FSGDynamicTextAssetBundleData& OutBundleData) const;
	virtual void BP_ExtractBundles_Implementation(const UObject* Provider,
		FSGDynamicTextAssetBundleData& OutBundleData) const { }

	/**
	 * Blueprint hook for additional bundle serialization logic.
	 * Called by NotifySerializeBundles after Native_SerializeBundles.
	 * Default: no-op.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Asset Bundles",
		meta = (DisplayName = "Serialize Bundles"))
	void BP_SerializeBundles(const FSGDynamicTextAssetBundleData& BundleData,
		UPARAM(ref) FString& InOutSerializedContent) const;
	virtual void BP_SerializeBundles_Implementation(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutSerializedContent) const { }

	/**
	 * Blueprint hook for additional bundle deserialization logic.
	 * Called by NotifyDeserializeBundles after Native_DeserializeBundles.
	 * Default: returns true (passthrough).
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Asset Bundles",
		meta = (DisplayName = "Deserialize Bundles"))
	bool BP_DeserializeBundles(const FString& SerializedContent,
		FSGDynamicTextAssetBundleData& OutBundleData) const;
	virtual bool BP_DeserializeBundles_Implementation(const FString& SerializedContent,
		FSGDynamicTextAssetBundleData& OutBundleData) const { return true; }
};
