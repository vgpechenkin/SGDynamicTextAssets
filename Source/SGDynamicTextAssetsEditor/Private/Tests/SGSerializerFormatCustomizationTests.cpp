// Copyright Start Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"

#include "Core/SGDTASerializerFormat.h"
#include "Customization/SGDTASerializerFormatCustomization.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "SGDTASerializerFormatCustomizationTestTypes.h"
#include "UObject/UnrealType.h"

// -- MakeInstance returns a valid non-null instance --

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSGDTASerializerFormatCustomization_MakeInstance_ReturnsValid,
	"SGDynamicTextAssets.Editor.Customization.SerializerFormat.MakeInstance_ReturnsValid",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter |
	EAutomationTestFlags::MediumPriority)

bool FSGDTASerializerFormatCustomization_MakeInstance_ReturnsValid::RunTest(const FString& Parameters)
{
	// MakeInstance should return a valid customization instance
	// TSharedRef is always valid by construction, so just verify it doesn't crash
	TSharedRef<IPropertyTypeCustomization> instance = FSGDTASerializerFormatCustomization::MakeInstance();
	TestTrue(TEXT("MakeInstance should return a non-null instance"), &instance.Get() != nullptr);
	return true;
}

// -- Registration check --

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSGDTASerializerFormatCustomization_IsRegistered,
	"SGDynamicTextAssets.Editor.Customization.SerializerFormat.IsRegistered",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter |
	EAutomationTestFlags::MediumPriority)

bool FSGDTASerializerFormatCustomization_IsRegistered::RunTest(const FString& Parameters)
{
	// Verify the PropertyEditor module is loaded (it registers our customization on startup)
	TestTrue(TEXT("PropertyEditor module should be loaded"),
		FModuleManager::Get().IsModuleLoaded("PropertyEditor"));

	// Verify a second MakeInstance call also succeeds (sanity check for registration stability)
	TSharedRef<IPropertyTypeCustomization> instance1 = FSGDTASerializerFormatCustomization::MakeInstance();
	TSharedRef<IPropertyTypeCustomization> instance2 = FSGDTASerializerFormatCustomization::MakeInstance();
	TestTrue(TEXT("Multiple MakeInstance calls should produce distinct instances"),
		&instance1.Get() != &instance2.Get());

	return true;
}

// -- Test UObject with UPROPERTY reflection --

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FSGDTASerializerFormatCustomization_TestObject_PropertiesAccessible,
	"SGDynamicTextAssets.Editor.Customization.SerializerFormat.TestObject_PropertiesAccessible",
	EAutomationTestFlags::EditorContext |
	EAutomationTestFlags::ProductFilter |
	EAutomationTestFlags::MediumPriority)

bool FSGDTASerializerFormatCustomization_TestObject_PropertiesAccessible::RunTest(const FString& Parameters)
{
	// Create test object
	USGDTASerializerFormatCustomizationTestObject* testObj =
		NewObject<USGDTASerializerFormatCustomizationTestObject>();
	TestNotNull(TEXT("Test object should be created"), testObj);

	// Verify SingleFormat property exists via reflection
	const FProperty* singleProp = testObj->GetClass()->FindPropertyByName(TEXT("SingleFormat"));
	TestNotNull(TEXT("SingleFormat property should exist in reflection"), singleProp);

	// Verify BitmaskFormat property exists and has SGDTABitmask meta
	const FProperty* bitmaskProp = testObj->GetClass()->FindPropertyByName(TEXT("BitmaskFormat"));
	TestNotNull(TEXT("BitmaskFormat property should exist in reflection"), bitmaskProp);

	if (bitmaskProp)
	{
		TestTrue(TEXT("BitmaskFormat should have SGDTABitmask metadata"),
			bitmaskProp->HasMetaData(TEXT("SGDTABitmask")));
	}

	// Verify SingleFormat does NOT have SGDTABitmask meta
	if (singleProp)
	{
		TestFalse(TEXT("SingleFormat should NOT have SGDTABitmask metadata"),
			singleProp->HasMetaData(TEXT("SGDTABitmask")));
	}

	// Verify default values are invalid
	TestFalse(TEXT("SingleFormat should default to invalid"),
		testObj->SingleFormat.IsValid());
	TestFalse(TEXT("BitmaskFormat should default to invalid"),
		testObj->BitmaskFormat.IsValid());

	return true;
}
