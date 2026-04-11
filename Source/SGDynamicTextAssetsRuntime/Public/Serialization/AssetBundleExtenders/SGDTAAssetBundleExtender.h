// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/SGDynamicTextAssetBundleData.h"
#include "Core/SGDTASerializerFormat.h"
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
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = "Start Games", DisplayName = "Asset Bundle Extender (Abstract Base)")
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
	 * Pre-serialization hook. Called before provider properties are serialized.
	 * Calls Native_PreSerialize followed by BP_PreSerialize.
	 *
	 * @param InOutContent The serialized content to pre-process
	 * @param Format The serializer format of the content
	 */
	void NotifyPreSerialize(FString& InOutContent, const FSGDTASerializerFormat& Format) const;

	/**
	 * Post-serialization hook. Called after provider properties are serialized.
	 * Writes bundle data into the serialized content string.
	 * Calls Native_PostSerialize followed by BP_PostSerialize.
	 *
	 * @param BundleData The bundle data to serialize
	 * @param InOutContent The full serialized file content to modify
	 * @param Format The serializer format of the content
	 */
	void NotifyPostSerialize(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutContent, const FSGDTASerializerFormat& Format) const;

	/**
	 * Pre-deserialization hook. Called before provider properties are deserialized.
	 * Extracts bundle metadata AND restores wrapped properties to plain format.
	 * Calls Native_PreDeserialize followed by BP_PreDeserialize.
	 *
	 * @param InOutContent The serialized content (mutable, extender may unwrap properties)
	 * @param OutBundleData Populated with extracted bundle metadata
	 * @param Format The serializer format of the content
	 * @return True if pre-deserialization succeeded
	 */
	bool NotifyPreDeserialize(FString& InOutContent,
		FSGDynamicTextAssetBundleData& OutBundleData, const FSGDTASerializerFormat& Format) const;

	/**
	 * Post-deserialization hook. Called after provider properties are deserialized.
	 * Calls Native_PostDeserialize followed by BP_PostDeserialize.
	 *
	 * @param Content The serialized content (read-only)
	 * @param InOutBundleData Bundle data to post-process
	 * @param Format The serializer format of the content
	 * @return True if post-deserialization succeeded
	 */
	bool NotifyPostDeserialize(const FString& Content,
		FSGDynamicTextAssetBundleData& InOutBundleData, const FSGDTASerializerFormat& Format) const;

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
	 * C++ implementation of pre-serialization.
	 * Default: no-op. Override to prepare content before property serialization.
	 */
	virtual void Native_PreSerialize(FString& InOutContent,
		const FSGDTASerializerFormat& Format) const;

	/**
	 * C++ implementation of post-serialization.
	 * Default: no-op. Subclasses override to inject bundle data into content.
	 */
	virtual void Native_PostSerialize(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutContent, const FSGDTASerializerFormat& Format) const;

	/**
	 * C++ implementation of pre-deserialization.
	 * Default: returns true (passthrough). Subclasses override to extract
	 * bundle metadata and restore wrapped properties to plain format.
	 */
	virtual bool Native_PreDeserialize(FString& InOutContent,
		FSGDynamicTextAssetBundleData& OutBundleData, const FSGDTASerializerFormat& Format) const;

	/**
	 * C++ implementation of post-deserialization.
	 * Default: returns true (passthrough). Override for post-processing
	 * after properties are deserialized.
	 */
	virtual bool Native_PostDeserialize(const FString& Content,
		FSGDynamicTextAssetBundleData& InOutBundleData, const FSGDTASerializerFormat& Format) const;

	/**
	 * Blueprint hook for bundle extraction.
	 * Called by NotifyExtractBundles after Native_ExtractBundles.
	 * Default: no-op.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Asset Bundles",
		meta = (DisplayName = "Extract Bundles"))
	void BP_ExtractBundles(const UObject* Provider,
		FSGDynamicTextAssetBundleData& OutBundleData) const;
	virtual void BP_ExtractBundles_Implementation(const UObject* Provider,
		FSGDynamicTextAssetBundleData& OutBundleData) const { }

	/** Blueprint hook for pre-serialization. Called after Native_PreSerialize. */
	UFUNCTION(BlueprintNativeEvent, Category = "Asset Bundles",
		meta = (DisplayName = "Pre Serialize"))
	void BP_PreSerialize(UPARAM(ref) FString& InOutContent,
		const FSGDTASerializerFormat& Format) const;
	virtual void BP_PreSerialize_Implementation(FString& InOutContent,
		const FSGDTASerializerFormat& Format) const { }

	/** Blueprint hook for post-serialization. Called after Native_PostSerialize. */
	UFUNCTION(BlueprintNativeEvent, Category = "Asset Bundles",
		meta = (DisplayName = "Post Serialize"))
	void BP_PostSerialize(const FSGDynamicTextAssetBundleData& BundleData,
		UPARAM(ref) FString& InOutContent, const FSGDTASerializerFormat& Format) const;
	virtual void BP_PostSerialize_Implementation(const FSGDynamicTextAssetBundleData& BundleData,
		FString& InOutContent, const FSGDTASerializerFormat& Format) const { }

	/** Blueprint hook for pre-deserialization. Called after Native_PreDeserialize. */
	UFUNCTION(BlueprintNativeEvent, Category = "Asset Bundles",
		meta = (DisplayName = "Pre Deserialize"))
	bool BP_PreDeserialize(UPARAM(ref) FString& InOutContent,
		FSGDynamicTextAssetBundleData& OutBundleData, const FSGDTASerializerFormat& Format) const;
	virtual bool BP_PreDeserialize_Implementation(FString& InOutContent,
		FSGDynamicTextAssetBundleData& OutBundleData, const FSGDTASerializerFormat& Format) const { return true; }

	/** Blueprint hook for post-deserialization. Called after Native_PostDeserialize. */
	UFUNCTION(BlueprintNativeEvent, Category = "Asset Bundles",
		meta = (DisplayName = "Post Deserialize"))
	bool BP_PostDeserialize(const FString& Content,
		FSGDynamicTextAssetBundleData& InOutBundleData, const FSGDTASerializerFormat& Format) const;
	virtual bool BP_PostDeserialize_Implementation(const FString& Content,
		FSGDynamicTextAssetBundleData& InOutBundleData, const FSGDTASerializerFormat& Format) const { return true; }
};
