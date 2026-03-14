// Copyright Start Games, Inc. All Rights Reserved.

#include "Serialization/SGDynamicTextAssetSerializer.h"

#if WITH_EDITOR
#include "Styling/AppStyle.h"
#endif

const FString ISGDynamicTextAssetSerializer::KEY_METADATA = TEXT("metadata");
const FString ISGDynamicTextAssetSerializer::KEY_TYPE = TEXT("type");
const FString ISGDynamicTextAssetSerializer::KEY_VERSION = TEXT("version");
const FString ISGDynamicTextAssetSerializer::KEY_ID = TEXT("id");
const FString ISGDynamicTextAssetSerializer::KEY_USER_FACING_ID = TEXT("userfacingid");
const FString ISGDynamicTextAssetSerializer::KEY_DATA = TEXT("data");
const FString ISGDynamicTextAssetSerializer::KEY_SGDT_ASSET_BUNDLES = TEXT("sgdtAssetBundles");

#if WITH_EDITOR
const FSlateBrush* ISGDynamicTextAssetSerializer::GetIconBrush() const
{
	// Using generic object as default to display something
	static const FSlateBrush* icon = FAppStyle::GetBrush("ClassIcon.Object");
	return icon;
}
#endif

bool ISGDynamicTextAssetSerializer::ExtractSGDTAssetBundles(const FString& InString, FSGDynamicTextAssetBundleData& OutBundleData) const
{
	// Default implementation does not extract bundles.
	// Format-specific serializers override this to parse their bundle block.
	return false;
}

FString ISGDynamicTextAssetSerializer::GetFormatName_String() const
{
	return GetFormatName().ToString();
}
