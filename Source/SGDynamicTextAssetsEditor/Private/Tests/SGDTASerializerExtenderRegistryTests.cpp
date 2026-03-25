// Copyright Start Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Core/SGDTAClassId.h"
#include "Management/SGDTASerializerExtenderRegistry.h"
#include "Serialization/SGDTASerializerExtenderBase.h"
#include "HAL/PlatformFileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_AddExtender_CanBeFound,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.AddExtender.CanBeFound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_AddExtender_CanBeFound::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> classPtr(USGDTASerializerExtenderBase::StaticClass());

	registry.AddExtender(id, classPtr);

	const FSGDTASerializerExtenderRegistryEntry* found = registry.FindByExtenderId(id);
	TestNotNull(TEXT("FindByExtenderId should return the added entry"), found);
	if (found)
	{
		TestTrue(TEXT("Found entry should have matching ExtenderId"), found->ExtenderId == id);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_AddExtender_FindByClassName,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.AddExtender.FindByClassName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_AddExtender_FindByClassName::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> classPtr(USGDTASerializerExtenderBase::StaticClass());

	registry.AddExtender(id, classPtr);

	const FSGDTASerializerExtenderRegistryEntry* found = registry.FindByClassName(TEXT("SGDTASerializerExtenderBase"));
	TestNotNull(TEXT("FindByClassName should return the added entry"), found);
	if (found)
	{
		TestTrue(TEXT("Found entry should have matching ExtenderId"), found->ExtenderId == id);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_RemoveExtender_NotFound,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.RemoveExtender.NotFound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_RemoveExtender_NotFound::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> classPtr(USGDTASerializerExtenderBase::StaticClass());

	registry.AddExtender(id, classPtr);
	registry.RemoveExtender(id);

	const FSGDTASerializerExtenderRegistryEntry* found = registry.FindByExtenderId(id);
	TestNull(TEXT("FindByExtenderId should return nullptr after removal"), found);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_RemoveExtender_ReturnValue,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.RemoveExtender.ReturnValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_RemoveExtender_ReturnValue::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> classPtr(USGDTASerializerExtenderBase::StaticClass());

	registry.AddExtender(id, classPtr);

	bool bRemoved = registry.RemoveExtender(id);
	TestTrue(TEXT("Removing existing extender should return true"), bRemoved);

	bool bRemovedAgain = registry.RemoveExtender(id);
	TestFalse(TEXT("Removing non-existing extender should return false"), bRemovedAgain);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetAllExtenders_ReturnsAll,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetAllExtenders.ReturnsAll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetAllExtenders_ReturnsAll::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	TSoftClassPtr<UObject> classPtr(USGDTASerializerExtenderBase::StaticClass());

	registry.AddExtender(FSGDTAClassId::NewGeneratedId(), classPtr);
	registry.AddExtender(FSGDTAClassId::NewGeneratedId(), classPtr);
	registry.AddExtender(FSGDTAClassId::NewGeneratedId(), classPtr);

	TestEqual(TEXT("GetAllExtenders should return 3 entries"), registry.GetAllExtenders().Num(), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_AddExtender_DuplicateIdUpdates,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.AddExtender.DuplicateIdUpdates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_AddExtender_DuplicateIdUpdates::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();

	TSoftClassPtr<UObject> classPtr1(USGDTASerializerExtenderBase::StaticClass());
	registry.AddExtender(id, classPtr1);

	// Re-add with different class (using UObject as a different class)
	TSoftClassPtr<UObject> classPtr2(UObject::StaticClass());
	registry.AddExtender(id, classPtr2);

	TestEqual(TEXT("Duplicate ID should not increase count"), registry.Num(), 1);

	const FSGDTASerializerExtenderRegistryEntry* found = registry.FindByExtenderId(id);
	TestNotNull(TEXT("Entry should still be found"), found);
	if (found)
	{
		TestEqual(TEXT("Entry should have updated class name"), found->ClassName, TEXT("Object"));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetSoftClassPtr_Valid,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetSoftClassPtr.Valid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetSoftClassPtr_Valid::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> classPtr(USGDTASerializerExtenderBase::StaticClass());

	registry.AddExtender(id, classPtr);

	TSoftClassPtr<UObject> resolved = registry.GetSoftClassPtr(id);
	TestFalse(TEXT("GetSoftClassPtr should return non-null for registered extender"), resolved.IsNull());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetSoftClassPtr_NotFound,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetSoftClassPtr.NotFound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetSoftClassPtr_NotFound::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId unknownId = FSGDTAClassId::NewGeneratedId();

	TSoftClassPtr<UObject> resolved = registry.GetSoftClassPtr(unknownId);
	TestTrue(TEXT("GetSoftClassPtr should return null for unknown ID"), resolved.IsNull());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_AddExtender_InvalidIdIgnored,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.AddExtender.InvalidIdIgnored",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_AddExtender_InvalidIdIgnored::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId invalidId;
	TSoftClassPtr<UObject> classPtr(USGDTASerializerExtenderBase::StaticClass());

	registry.AddExtender(invalidId, classPtr);

	TestEqual(TEXT("Invalid ID should be ignored, Num() should be 0"), registry.Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_IsDirty_AfterAdd,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.IsDirty.AfterAdd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_IsDirty_AfterAdd::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	TestFalse(TEXT("Fresh registry should not be dirty"), registry.IsDirty());

	registry.AddExtender(FSGDTAClassId::NewGeneratedId(), TSoftClassPtr<UObject>(USGDTASerializerExtenderBase::StaticClass()));
	TestTrue(TEXT("Registry should be dirty after AddExtender"), registry.IsDirty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_Clear_EmptiesAll,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.Clear.EmptiesAll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_Clear_EmptiesAll::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();
	registry.AddExtender(id, TSoftClassPtr<UObject>(USGDTASerializerExtenderBase::StaticClass()));

	registry.Clear();

	TestEqual(TEXT("Num() should be 0 after Clear"), registry.Num(), 0);
	TestNull(TEXT("FindByExtenderId should return nullptr after Clear"), registry.FindByExtenderId(id));

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_SaveLoad_Roundtrip,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.SaveLoad.Roundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_SaveLoad_Roundtrip::RunTest(const FString& Parameters)
{
	FString tempFile = FPaths::CreateTempFilename(*FPaths::ProjectSavedDir(), TEXT("ExtRegTest"), TEXT(".json"));

	// Save
	FSGDTASerializerExtenderRegistry saveRegistry;
	FSGDTAClassId id1 = FSGDTAClassId::NewGeneratedId();
	FSGDTAClassId id2 = FSGDTAClassId::NewGeneratedId();
	saveRegistry.AddExtender(id1, TSoftClassPtr<UObject>(USGDTASerializerExtenderBase::StaticClass()));
	saveRegistry.AddExtender(id2, TSoftClassPtr<UObject>(UObject::StaticClass()));

	bool bSaved = saveRegistry.SaveToFile(tempFile);
	TestTrue(TEXT("SaveToFile should succeed"), bSaved);

	// Load into a new registry
	FSGDTASerializerExtenderRegistry loadRegistry;
	bool bLoaded = loadRegistry.LoadFromFile(tempFile);
	TestTrue(TEXT("LoadFromFile should succeed"), bLoaded);

	TestEqual(TEXT("Loaded registry should have same number of entries"), loadRegistry.Num(), 2);

	const FSGDTASerializerExtenderRegistryEntry* found1 = loadRegistry.FindByExtenderId(id1);
	TestNotNull(TEXT("First extender should be found after load"), found1);

	const FSGDTASerializerExtenderRegistryEntry* found2 = loadRegistry.FindByExtenderId(id2);
	TestNotNull(TEXT("Second extender should be found after load"), found2);

	// Clean up temp file
	IFileManager::Get().Delete(*tempFile);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_LoadFromFile_InvalidSchema,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.LoadFromFile.InvalidSchema",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_LoadFromFile_InvalidSchema::RunTest(const FString& Parameters)
{
	// Expect the error log from the registry when schema validation fails
	AddExpectedError(TEXT("Invalid schema in registry"), EAutomationExpectedErrorFlags::Contains, 1);

	FString tempFile = FPaths::CreateTempFilename(*FPaths::ProjectSavedDir(), TEXT("ExtRegTest"), TEXT(".json"));

	// Write invalid schema JSON
	FString badJson = TEXT("{\"schema\": \"wrong_schema\", \"version\": 1, \"extenders\": []}");
	FFileHelper::SaveStringToFile(badJson, *tempFile);

	FSGDTASerializerExtenderRegistry registry;
	bool bLoaded = registry.LoadFromFile(tempFile);
	TestFalse(TEXT("LoadFromFile should fail with invalid schema"), bLoaded);

	// Clean up
	IFileManager::Get().Delete(*tempFile);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetClass_ResolvesViaRegistry,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetClass.ResolvesViaRegistry",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetClass_ResolvesViaRegistry::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> classPtr(USGDTASerializerExtenderBase::StaticClass());

	registry.AddExtender(id, classPtr);

	// Use the template GetClass
	TSoftClassPtr<UObject> resolved = id.GetClass<FSGDTASerializerExtenderRegistry>(registry);
	TestFalse(TEXT("GetClass<Registry> should return non-null for registered ID"), resolved.IsNull());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetClass_InvalidIdReturnsEmpty,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetClass.InvalidIdReturnsEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetClass_InvalidIdReturnsEmpty::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId invalidId;

	TSoftClassPtr<UObject> resolved = invalidId.GetClass<FSGDTASerializerExtenderRegistry>(registry);
	TestTrue(TEXT("GetClass with invalid ID should return empty"), resolved.IsNull());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetClass_TypedCast,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetClass.TypedCast",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetClass_TypedCast::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry registry;
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> classPtr(USGDTASerializerExtenderBase::StaticClass());

	registry.AddExtender(id, classPtr);

	// Use the typed template GetClass
	TSoftClassPtr<USGDTASerializerExtenderBase> resolved =
		id.GetClassTyped<USGDTASerializerExtenderBase, FSGDTASerializerExtenderRegistry>(registry);
	TestFalse(TEXT("Typed GetClass should return non-null for registered ID"), resolved.IsNull());

	return true;
}
