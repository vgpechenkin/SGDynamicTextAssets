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

void USGDTAAssetBundleExtender::NotifyPreSerialize(FString& InOutContent,
	const FSGDTASerializerFormat& Format) const
{
	Native_PreSerialize(InOutContent, Format);
	BP_PreSerialize(InOutContent, Format);
}

void USGDTAAssetBundleExtender::NotifyPostSerialize(const FSGDynamicTextAssetBundleData& BundleData,
	FString& InOutContent, const FSGDTASerializerFormat& Format) const
{
	Native_PostSerialize(BundleData, InOutContent, Format);
	BP_PostSerialize(BundleData, InOutContent, Format);
}

bool USGDTAAssetBundleExtender::NotifyPreDeserialize(FString& InOutContent,
	FSGDynamicTextAssetBundleData& OutBundleData, const FSGDTASerializerFormat& Format) const
{
	const bool bNativeResult = Native_PreDeserialize(InOutContent, OutBundleData, Format);
	const bool bBPResult = BP_PreDeserialize(InOutContent, OutBundleData, Format);
	return bNativeResult && bBPResult;
}

bool USGDTAAssetBundleExtender::NotifyPostDeserialize(const FString& Content,
	FSGDynamicTextAssetBundleData& InOutBundleData, const FSGDTASerializerFormat& Format) const
{
	const bool bNativeResult = Native_PostDeserialize(Content, InOutBundleData, Format);
	const bool bBPResult = BP_PostDeserialize(Content, InOutBundleData, Format);
	return bNativeResult && bBPResult;
}

void USGDTAAssetBundleExtender::Native_ExtractBundles(const UObject* Provider,
	FSGDynamicTextAssetBundleData& OutBundleData) const
{
#if WITH_EDITORONLY_DATA
	OutBundleData.ExtractFromObject(Provider);
#endif
}

void USGDTAAssetBundleExtender::Native_PreSerialize(FString& InOutContent,
	const FSGDTASerializerFormat& Format) const
{
	// No-op default. Override to prepare content before property serialization.
}

void USGDTAAssetBundleExtender::Native_PostSerialize(const FSGDynamicTextAssetBundleData& BundleData,
	FString& InOutContent, const FSGDTASerializerFormat& Format) const
{
	// No-op default. Concrete subclasses override to inject bundle data.
}

bool USGDTAAssetBundleExtender::Native_PreDeserialize(FString& InOutContent,
	FSGDynamicTextAssetBundleData& OutBundleData, const FSGDTASerializerFormat& Format) const
{
	// No-op default. Concrete subclasses override to extract bundles and unwrap content.
	return true;
}

bool USGDTAAssetBundleExtender::Native_PostDeserialize(const FString& Content,
	FSGDynamicTextAssetBundleData& InOutBundleData, const FSGDTASerializerFormat& Format) const
{
	// No-op default. Override for post-deserialization processing.
	return true;
}
