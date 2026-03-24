// Copyright Start Games, Inc. All Rights Reserved.

#include "SGDTAAssetBundleExtenderTests.h"

#include "Misc/AutomationTest.h"
#include "Core/ISGDynamicTextAssetProvider.h"
#include "Core/SGDTASerializerFormat.h"
#include "Management/SGDynamicTextAssetRegistry.h"
#include "Settings/SGDynamicTextAssetSettings.h"
#include "Statics/SGDynamicTextAssetStatics.h"
#include "Tests/SGDynamicTextAssetBundleTestTypes.h"

#if WITH_EDITOR
#include "Misc/DataValidation.h"
#endif

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetBundleExtender_GetOrCreate_NullClassReturnsNull,
	"SGDynamicTextAssets.Runtime.Serialization.AssetBundleExtender.GetOrCreate.NullClassReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAssetBundleExtender_GetOrCreate_NullClassReturnsNull::RunTest(const FString& Parameters)
{
	USGDynamicTextAssetRegistry* registry = USGDynamicTextAssetRegistry::Get();
	TestNotNull(TEXT("Registry should be available"), registry);
	if (!registry)
	{
		return false;
	}

	AddExpectedError(TEXT("Inputted NULL ExtenderClass"), EAutomationExpectedErrorFlags::Contains, 1);

	TSoftClassPtr<USGDTAAssetBundleExtender> nullClass;
	USGDTAAssetBundleExtender* result = registry->GetOrCreateAssetBundleExtender(nullClass);
	TestNull(TEXT("Null class should return null instance"), result);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetBundleExtender_GetOrCreate_CreatesInstance,
	"SGDynamicTextAssets.Runtime.Serialization.AssetBundleExtender.GetOrCreate.CreatesInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAssetBundleExtender_GetOrCreate_CreatesInstance::RunTest(const FString& Parameters)
{
	USGDynamicTextAssetRegistry* registry = USGDynamicTextAssetRegistry::Get();
	TestNotNull(TEXT("Registry should be available"), registry);
	if (!registry)
	{
		return false;
	}

	TSoftClassPtr<USGDTAAssetBundleExtender> classPtr(USGDTATestAssetBundleExtenderA::StaticClass());
	USGDTAAssetBundleExtender* result = registry->GetOrCreateAssetBundleExtender(classPtr);
	TestNotNull(TEXT("Should create a valid instance"), result);

	if (result)
	{
		TestTrue(TEXT("Instance should be of the correct class"),
			result->IsA(USGDTATestAssetBundleExtenderA::StaticClass()));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetBundleExtender_GetOrCreate_CacheReturnsSameInstance,
	"SGDynamicTextAssets.Runtime.Serialization.AssetBundleExtender.GetOrCreate.CacheReturnsSameInstance",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAssetBundleExtender_GetOrCreate_CacheReturnsSameInstance::RunTest(const FString& Parameters)
{
	USGDynamicTextAssetRegistry* registry = USGDynamicTextAssetRegistry::Get();
	TestNotNull(TEXT("Registry should be available"), registry);
	if (!registry)
	{
		return false;
	}

	TSoftClassPtr<USGDTAAssetBundleExtender> classPtr(USGDTATestAssetBundleExtenderA::StaticClass());
	USGDTAAssetBundleExtender* first = registry->GetOrCreateAssetBundleExtender(classPtr);
	USGDTAAssetBundleExtender* second = registry->GetOrCreateAssetBundleExtender(classPtr);

	TestNotNull(TEXT("First call should return valid instance"), first);
	TestNotNull(TEXT("Second call should return valid instance"), second);
	TestEqual(TEXT("Both calls should return the same cached instance"), first, second);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetBundleExtender_GetOrCreate_DifferentClassesDifferentInstances,
	"SGDynamicTextAssets.Runtime.Serialization.AssetBundleExtender.GetOrCreate.DifferentClassesDifferentInstances",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAssetBundleExtender_GetOrCreate_DifferentClassesDifferentInstances::RunTest(const FString& Parameters)
{
	USGDynamicTextAssetRegistry* registry = USGDynamicTextAssetRegistry::Get();
	TestNotNull(TEXT("Registry should be available"), registry);
	if (!registry)
	{
		return false;
	}

	TSoftClassPtr<USGDTAAssetBundleExtender> classPtrA(USGDTATestAssetBundleExtenderA::StaticClass());
	TSoftClassPtr<USGDTAAssetBundleExtender> classPtrB(USGDTATestAssetBundleExtenderB::StaticClass());

	USGDTAAssetBundleExtender* instanceA = registry->GetOrCreateAssetBundleExtender(classPtrA);
	USGDTAAssetBundleExtender* instanceB = registry->GetOrCreateAssetBundleExtender(classPtrB);

	TestNotNull(TEXT("Instance A should be valid"), instanceA);
	TestNotNull(TEXT("Instance B should be valid"), instanceB);
	TestNotEqual(TEXT("Different classes should produce different instances"), instanceA, instanceB);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetBundleExtender_Resolve_NullProviderReturnsNull,
	"SGDynamicTextAssets.Runtime.Serialization.AssetBundleExtender.Resolve.NullProviderReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAssetBundleExtender_Resolve_NullProviderReturnsNull::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("Inputted NULL Provider"), EAutomationExpectedErrorFlags::Contains, 1);

	TScriptInterface<ISGDynamicTextAssetProvider> nullProvider;
	FSGDTASerializerFormat format = FSGDTASerializerFormat::FromTypeId(1);

	TSoftClassPtr<USGDTAAssetBundleExtender> result =
		USGDynamicTextAssetStatics::ResolveAssetBundleExtender(nullProvider, format);

	TestTrue(TEXT("Null provider should resolve to null class"), result.IsNull());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetBundleExtender_Resolve_PerDTAOverrideTakesPriority,
	"SGDynamicTextAssets.Runtime.Serialization.AssetBundleExtender.Resolve.PerDTAOverrideTakesPriority",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAssetBundleExtender_Resolve_PerDTAOverrideTakesPriority::RunTest(const FString& Parameters)
{
	USGDTABundleTestDynamicTextAsset* asset = NewObject<USGDTABundleTestDynamicTextAsset>();
	TestNotNull(TEXT("Test asset should be created"), asset);
	if (!asset)
	{
		return false;
	}

	// Set per-DTA override to ExtenderA via property reflection (protected member)
	{
		const FProperty* prop = USGDTABundleTestDynamicTextAsset::StaticClass()->FindPropertyByName(
			TEXT("AssetBundleExtenderOverride"));
		TestNotNull(TEXT("AssetBundleExtenderOverride property should exist"), prop);
		if (prop)
		{
			TSoftClassPtr<USGDTAAssetBundleExtender> overrideValue(
				USGDTATestAssetBundleExtenderA::StaticClass());
			prop->CopyCompleteValue(prop->ContainerPtrToValuePtr<void>(asset), &overrideValue);
		}
	}

	TScriptInterface<ISGDynamicTextAssetProvider> provider(asset);
	FSGDTASerializerFormat format = FSGDTASerializerFormat::FromTypeId(1);

	TSoftClassPtr<USGDTAAssetBundleExtender> result =
		USGDynamicTextAssetStatics::ResolveAssetBundleExtender(provider, format);

	TestFalse(TEXT("Should resolve to a non-null class"), result.IsNull());

	if (!result.IsNull())
	{
		UClass* loadedClass = result.LoadSynchronous();
		TestEqual(TEXT("Should resolve to ExtenderA"),
			loadedClass, USGDTATestAssetBundleExtenderA::StaticClass());
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetBundleExtender_Resolve_NoMatchReturnsNull,
	"SGDynamicTextAssets.Runtime.Serialization.AssetBundleExtender.Resolve.NoMatchReturnsNull",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAssetBundleExtender_Resolve_NoMatchReturnsNull::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("No asset bundle extender found"), EAutomationExpectedErrorFlags::Contains, 1);

	USGDTABundleTestDynamicTextAsset* asset = NewObject<USGDTABundleTestDynamicTextAsset>();
	TestNotNull(TEXT("Test asset should be created"), asset);
	if (!asset)
	{
		return false;
	}

	// No per-DTA override set, use a format unlikely to be in settings
	TScriptInterface<ISGDynamicTextAssetProvider> provider(asset);
	FSGDTASerializerFormat unusedFormat = FSGDTASerializerFormat::FromTypeId(31);

	TSoftClassPtr<USGDTAAssetBundleExtender> result =
		USGDynamicTextAssetStatics::ResolveAssetBundleExtender(provider, unusedFormat);

	TestTrue(TEXT("Should resolve to null when no mapping matches"), result.IsNull());

	return true;
}

#if WITH_EDITOR

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetBundleExtender_Validation_FailsWithNullExtenderClass,
	"SGDynamicTextAssets.Runtime.Serialization.AssetBundleExtender.Validation.FailsWithNullExtenderClass",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAssetBundleExtender_Validation_FailsWithNullExtenderClass::RunTest(const FString& Parameters)
{
	USGDynamicTextAssetSettingsAsset* settings = NewObject<USGDynamicTextAssetSettingsAsset>();
	TestNotNull(TEXT("Settings asset should be created"), settings);
	if (!settings)
	{
		return false;
	}

	// Add a mapping with null ExtenderClass
	FSGAssetBundleExtenderMapping mapping;
	mapping.AppliesTo = FSGDTASerializerFormat::FromTypeId(1);
	mapping.ExtenderClass = nullptr;
	settings->AssetBundleExtenderMappings.Add(mapping);

	FDataValidationContext context;
	EDataValidationResult result = settings->IsDataValid(context);

	TestEqual(TEXT("Validation should fail with null extender class"),
		result, EDataValidationResult::Invalid);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FAssetBundleExtender_Validation_FailsWithDuplicateFormat,
	"SGDynamicTextAssets.Runtime.Serialization.AssetBundleExtender.Validation.FailsWithDuplicateFormat",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FAssetBundleExtender_Validation_FailsWithDuplicateFormat::RunTest(const FString& Parameters)
{
	USGDynamicTextAssetSettingsAsset* settings = NewObject<USGDynamicTextAssetSettingsAsset>();
	TestNotNull(TEXT("Settings asset should be created"), settings);
	if (!settings)
	{
		return false;
	}

	// Add two mappings both covering format 1 (bit 1 << 1 = 2)
	FSGAssetBundleExtenderMapping mappingA;
	mappingA.AppliesTo = FSGDTASerializerFormat::FromTypeId(1 << 1);
	mappingA.ExtenderClass = TSoftClassPtr<USGDTAAssetBundleExtender>(
		USGDTATestAssetBundleExtenderA::StaticClass());
	settings->AssetBundleExtenderMappings.Add(mappingA);

	FSGAssetBundleExtenderMapping mappingB;
	mappingB.AppliesTo = FSGDTASerializerFormat::FromTypeId(1 << 1);
	mappingB.ExtenderClass = TSoftClassPtr<USGDTAAssetBundleExtender>(
		USGDTATestAssetBundleExtenderB::StaticClass());
	settings->AssetBundleExtenderMappings.Add(mappingB);

	FDataValidationContext context;
	EDataValidationResult result = settings->IsDataValid(context);

	TestEqual(TEXT("Validation should fail with duplicate format coverage"),
		result, EDataValidationResult::Invalid);

	return true;
}

#endif
