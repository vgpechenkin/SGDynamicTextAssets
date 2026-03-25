// Copyright Start Games, Inc. All Rights Reserved.

#include "Serialization/AssetBundleExtenders/SGDTAAssetBundleExtender.h"

#include "SGDynamicTextAssetLogs.h"

void USGDTAAssetBundleExtender::NotifyExtractBundles(const UObject* Provider,
	FSGDynamicTextAssetBundleData& OutBundleData) const
{
	if (!Provider)
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
			TEXT("USGDTAAssetBundleExtender::NotifyExtractBundles: Provider is null, skipping extraction"));
		return;
	}

	Native_ExtractBundles(Provider, OutBundleData);
	BP_ExtractBundles(Provider, OutBundleData);
}

void USGDTAAssetBundleExtender::NotifySerializeBundles(const FSGDynamicTextAssetBundleData& BundleData,
	FString& InOutSerializedContent, const FSGDTASerializerFormat& Format) const
{
	Native_SerializeBundles(BundleData, InOutSerializedContent, Format);
	BP_SerializeBundles(BundleData, InOutSerializedContent, Format);
}

bool USGDTAAssetBundleExtender::NotifyDeserializeBundles(const FString& SerializedContent,
	FSGDynamicTextAssetBundleData& OutBundleData, const FSGDTASerializerFormat& Format) const
{
	const bool bNativeResult = Native_DeserializeBundles(SerializedContent, OutBundleData, Format);
	const bool bBPResult = BP_DeserializeBundles(SerializedContent, OutBundleData, Format);

	return bNativeResult && bBPResult;
}

void USGDTAAssetBundleExtender::Native_ExtractBundles(const UObject* Provider,
	FSGDynamicTextAssetBundleData& OutBundleData) const
{
#if WITH_EDITORONLY_DATA
	OutBundleData.ExtractFromObject(Provider);
#endif
}

void USGDTAAssetBundleExtender::Native_SerializeBundles(const FSGDynamicTextAssetBundleData& BundleData,
	FString& InOutSerializedContent, const FSGDTASerializerFormat& Format) const
{
	// No-op default. Concrete subclasses must override to produce output.
}

bool USGDTAAssetBundleExtender::Native_DeserializeBundles(const FString& SerializedContent,
	FSGDynamicTextAssetBundleData& OutBundleData, const FSGDTASerializerFormat& Format) const
{
	// No-op default. Concrete subclasses must override.
	return true;
}
