// Copyright Start Games, Inc. All Rights Reserved.

#include "Serialization/SGDTAAssetBundleExtender.h"

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
	FString& InOutSerializedContent) const
{
	Native_SerializeBundles(BundleData, InOutSerializedContent);
	BP_SerializeBundles(BundleData, InOutSerializedContent);
}

bool USGDTAAssetBundleExtender::NotifyDeserializeBundles(const FString& SerializedContent,
	FSGDynamicTextAssetBundleData& OutBundleData) const
{
	const bool bNativeResult = Native_DeserializeBundles(SerializedContent, OutBundleData);
	const bool bBPResult = BP_DeserializeBundles(SerializedContent, OutBundleData);

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
	FString& InOutSerializedContent) const
{
	// No-op default. Concrete subclasses must override to produce output.
}

bool USGDTAAssetBundleExtender::Native_DeserializeBundles(const FString& SerializedContent,
	FSGDynamicTextAssetBundleData& OutBundleData) const
{
	// No-op default. Concrete subclasses must override.
	return true;
}
