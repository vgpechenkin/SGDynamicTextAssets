// Copyright Start Games, Inc. All Rights Reserved.

#include "Settings/SGDynamicTextAssetSettings.h"

#include "Engine/AssetManager.h"
#include "Management/SGDynamicTextAssetFileManager.h"
#include "Serialization/SGDynamicTextAssetSerializer.h"
#include "Serialization/AssetBundleExtenders/SGDTADefaultAssetBundleExtender.h"
#include "SGDynamicTextAssetLogs.h"
#include "Statics/SGDynamicTextAssetConstants.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

USGDynamicTextAssetSettingsAsset::USGDynamicTextAssetSettingsAsset()
{
	// Default mapping: USGDTADefaultAssetBundleExtender covers all built-in formats
	const uint32 allFormatsBitmask =
		(1u << SGDynamicTextAssetConstants::JSON_SERIALIZER_TYPE_ID) |
		(1u << SGDynamicTextAssetConstants::XML_SERIALIZER_TYPE_ID) |
		(1u << SGDynamicTextAssetConstants::YAML_SERIALIZER_TYPE_ID);

	FSGAssetBundleExtenderMapping defaultMapping;

	defaultMapping.AppliesTo = FSGDTASerializerFormat::FromTypeId(
		(1u << SGDynamicTextAssetConstants::JSON_SERIALIZER_TYPE_ID)
		| (1u << SGDynamicTextAssetConstants::XML_SERIALIZER_TYPE_ID)
		| (1u << SGDynamicTextAssetConstants::YAML_SERIALIZER_TYPE_ID));

	defaultMapping.ExtenderClass = TSoftClassPtr<USGDTAAssetBundleExtender>(
		USGDTADefaultAssetBundleExtender::StaticClass());

	AssetBundleExtenderMappings.Add(defaultMapping);
}

#if WITH_EDITOR
EDataValidationResult USGDynamicTextAssetSettingsAsset::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult result = Super::IsDataValid(Context);

	// Validate that every registered serializer format has an asset bundle extender
	TArray<TSharedPtr<ISGDynamicTextAssetSerializer>> serializers =
		FSGDynamicTextAssetFileManager::GetAllRegisteredSerializers();

	// Track which formats are covered and check for overlaps
	TMap<uint32, uint32> formatCoverageCount;

	for (const FSGAssetBundleExtenderMapping& mapping : AssetBundleExtenderMappings)
	{
		// Validate the extender class is set
		if (mapping.ExtenderClass.IsNull())
		{
			Context.AddError(INVTEXT("Asset bundle extender mapping has a null ExtenderClass. Each mapping must specify an extender."));
			result = EDataValidationResult::Invalid;
		}

		// Check which formats this mapping covers
		const uint32 bitmask = mapping.AppliesTo.GetTypeId();
		for (const TSharedPtr<ISGDynamicTextAssetSerializer>& serializer : serializers)
		{
			if (!serializer.IsValid())
			{
				continue;
			}

			const uint32 formatBit = 1u << serializer->GetSerializerFormat().GetTypeId();
			if ((bitmask & formatBit) != 0)
			{
				formatCoverageCount.FindOrAdd(serializer->GetSerializerFormat().GetTypeId())++;
			}
		}
	}

	// Check for formats with no coverage
	for (const TSharedPtr<ISGDynamicTextAssetSerializer>& serializer : serializers)
	{
		if (!serializer.IsValid())
		{
			continue;
		}

		const uint32 typeId = serializer->GetSerializerFormat().GetTypeId();
		const uint32* countPtr = formatCoverageCount.Find(typeId);

		if (!countPtr)
		{
			Context.AddError(FText::Format(
				INVTEXT("Serializer format `{0}` could not be found due to unknown error."),
				serializer->GetFormatName()));
			result = EDataValidationResult::Invalid;
			continue;
		}
		const uint32& count = *countPtr;

		if (count == 0)
		{
			Context.AddError(FText::Format(
				INVTEXT("Serializer format '{0}' has no asset bundle extender assigned. Add a mapping that covers this format."),
				serializer->GetFormatName()));
			result = EDataValidationResult::Invalid;
		}
		else if (count > 1)
		{
			Context.AddError(FText::Format(
				INVTEXT("Serializer format '{0}' is assigned to multiple asset bundle extenders. Only one extender per format is allowed."),
				serializer->GetFormatName()));
			result = EDataValidationResult::Invalid;
		}
	}

	return result;
}
#endif

FName USGDynamicTextAssetSettingsAsset::GetCustomCompressionName() const
{
	return CustomCompressionName;
}

USGDynamicTextAssetSettings* USGDynamicTextAssetSettings::Get()
{
	return GetMutableDefault<USGDynamicTextAssetSettings>();
}

USGDynamicTextAssetSettingsAsset* USGDynamicTextAssetSettings::GetSettings()
{
	USGDynamicTextAssetSettings* settings = Get();
	return settings ? settings->GetSettingsAsset() : nullptr;
}

USGDynamicTextAssetSettingsAsset* USGDynamicTextAssetSettings::GetSettingsAsset() const
{
	// Return cached asset if valid
	if (CachedSettingsAsset.IsValid())
	{
		return CachedSettingsAsset.Get();
	}

	// Try to load from soft reference
	if (!SettingsAsset.IsNull())
	{
		if (USGDynamicTextAssetSettingsAsset* loadedAsset = UAssetManager::Get().GetStreamableManager().LoadSynchronous<USGDynamicTextAssetSettingsAsset>(SettingsAsset))
		{
			CachedSettingsAsset = loadedAsset;
			return loadedAsset;
		}
	}

	UE_LOG(LogSGDynamicTextAssetsRuntime, Warning, TEXT("SGDynamicTextAssetSettings: Failed to load SettingsAsset(%s)"), *SettingsAsset.ToString());
	return nullptr;
}
