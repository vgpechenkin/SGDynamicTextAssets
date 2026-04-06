// Copyright Start Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Core/SGDTAClassId.h"
#include "Management/SGDTASerializerExtenderRegistry.h"
#include "Management/SGDTAExtenderManifest.h"
#include "Management/SGDTAExtenderManifestBinaryHeader.h"
#include "Serialization/SGDTASerializerExtenderBase.h"
#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace SGDTAExtenderRegistryTestHelpers
{
	static const FName TEST_FRAMEWORK_KEY = FName(TEXT("TestFramework"));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_AddExtender_CanBeFound,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.AddExtender.CanBeFound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_AddExtender_CanBeFound::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId Id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> ClassPtr(USGDTASerializerExtenderBase::StaticClass());

	Manifest->AddExtender(Id, ClassPtr);

	const FSGDTASerializerExtenderRegistryEntry* Found = Manifest->FindByExtenderId(Id);
	TestNotNull(TEXT("FindByExtenderId should return the added entry"), Found);
	if (Found)
	{
		TestTrue(TEXT("Found entry should have matching ExtenderId"), Found->ExtenderId == Id);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_AddExtender_FindByClassName,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.AddExtender.FindByClassName",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_AddExtender_FindByClassName::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId Id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> ClassPtr(USGDTASerializerExtenderBase::StaticClass());

	Manifest->AddExtender(Id, ClassPtr);

	const FSGDTASerializerExtenderRegistryEntry* Found = Manifest->FindByClassName(TEXT("SGDTASerializerExtenderBase"));
	TestNotNull(TEXT("FindByClassName should return the added entry"), Found);
	if (Found)
	{
		TestTrue(TEXT("Found entry should have matching ExtenderId"), Found->ExtenderId == Id);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_RemoveExtender_NotFound,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.RemoveExtender.NotFound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_RemoveExtender_NotFound::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId Id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> ClassPtr(USGDTASerializerExtenderBase::StaticClass());

	Manifest->AddExtender(Id, ClassPtr);
	Manifest->RemoveExtender(Id);

	const FSGDTASerializerExtenderRegistryEntry* Found = Manifest->FindByExtenderId(Id);
	TestNull(TEXT("FindByExtenderId should return nullptr after removal"), Found);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_RemoveExtender_ReturnValue,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.RemoveExtender.ReturnValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_RemoveExtender_ReturnValue::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId Id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> ClassPtr(USGDTASerializerExtenderBase::StaticClass());

	Manifest->AddExtender(Id, ClassPtr);

	bool bRemoved = Manifest->RemoveExtender(Id);
	TestTrue(TEXT("Removing existing extender should return true"), bRemoved);

	bool bRemovedAgain = Manifest->RemoveExtender(Id);
	TestFalse(TEXT("Removing non-existing extender should return false"), bRemovedAgain);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetAllExtenders_ReturnsAll,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetAllExtenders.ReturnsAll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetAllExtenders_ReturnsAll::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	TSoftClassPtr<UObject> ClassPtr(USGDTASerializerExtenderBase::StaticClass());

	Manifest->AddExtender(FSGDTAClassId::NewGeneratedId(), ClassPtr);
	Manifest->AddExtender(FSGDTAClassId::NewGeneratedId(), ClassPtr);
	Manifest->AddExtender(FSGDTAClassId::NewGeneratedId(), ClassPtr);

	TestEqual(TEXT("GetAllExtenders should return 3 entries"), Manifest->GetAllExtenders().Num(), 3);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_AddExtender_DuplicateIdUpdates,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.AddExtender.DuplicateIdUpdates",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_AddExtender_DuplicateIdUpdates::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId Id = FSGDTAClassId::NewGeneratedId();

	TSoftClassPtr<UObject> ClassPtr1(USGDTASerializerExtenderBase::StaticClass());
	Manifest->AddExtender(Id, ClassPtr1);

	// Re-add with different class (using UObject as a different class)
	TSoftClassPtr<UObject> ClassPtr2(UObject::StaticClass());
	Manifest->AddExtender(Id, ClassPtr2);

	TestEqual(TEXT("Duplicate ID should not increase count"), Manifest->Num(), 1);

	const FSGDTASerializerExtenderRegistryEntry* Found = Manifest->FindByExtenderId(Id);
	TestNotNull(TEXT("Entry should still be found"), Found);
	if (Found)
	{
		TestEqual(TEXT("Entry should have updated class name"), Found->ClassName, TEXT("Object"));
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetSoftClassPtr_Valid,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetSoftClassPtr.Valid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetSoftClassPtr_Valid::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId Id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> ClassPtr(USGDTASerializerExtenderBase::StaticClass());

	Manifest->AddExtender(Id, ClassPtr);

	TSoftClassPtr<UObject> Resolved = Manifest->GetSoftClassPtr(Id);
	TestFalse(TEXT("GetSoftClassPtr should return non-null for registered extender"), Resolved.IsNull());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetSoftClassPtr_NotFound,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetSoftClassPtr.NotFound",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetSoftClassPtr_NotFound::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId UnknownId = FSGDTAClassId::NewGeneratedId();

	TSoftClassPtr<UObject> Resolved = Manifest->GetSoftClassPtr(UnknownId);
	TestTrue(TEXT("GetSoftClassPtr should return null for unknown ID"), Resolved.IsNull());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_AddExtender_InvalidIdIgnored,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.AddExtender.InvalidIdIgnored",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_AddExtender_InvalidIdIgnored::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("Ignoring entry with invalid ExtenderId"), EAutomationExpectedErrorFlags::Contains, 1);

	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId InvalidId;
	TSoftClassPtr<UObject> ClassPtr(USGDTASerializerExtenderBase::StaticClass());

	Manifest->AddExtender(InvalidId, ClassPtr);

	TestEqual(TEXT("Invalid ID should be ignored, Num() should be 0"), Manifest->Num(), 0);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_IsDirty_AfterAdd,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.IsDirty.AfterAdd",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_IsDirty_AfterAdd::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TestFalse(TEXT("Fresh registry should not have any dirty manifests"), Registry.HasAnyDirty());

	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	TestFalse(TEXT("Fresh manifest should not be dirty"), Manifest->IsDirty());

	Manifest->AddExtender(FSGDTAClassId::NewGeneratedId(), TSoftClassPtr<UObject>(USGDTASerializerExtenderBase::StaticClass()));
	TestTrue(TEXT("Manifest should be dirty after AddExtender"), Manifest->IsDirty());
	TestTrue(TEXT("Registry should report dirty after manifest modification"), Registry.HasAnyDirty());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_ClearAllManifests_EmptiesAll,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.ClearAllManifests.EmptiesAll",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_ClearAllManifests_EmptiesAll::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTASerializerExtenderRegistry Registry;
	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(TEST_FRAMEWORK_KEY);
	Manifest->AddExtender(FSGDTAClassId::NewGeneratedId(), TSoftClassPtr<UObject>(USGDTASerializerExtenderBase::StaticClass()));

	Registry.ClearAllManifests();

	TestEqual(TEXT("NumManifests should be 0 after ClearAllManifests"), Registry.NumManifests(), 0);
	TestNull(TEXT("GetManifest should return nullptr after ClearAllManifests"),
		Registry.GetManifest(TEST_FRAMEWORK_KEY).Get());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_SaveLoadAllManifests_Roundtrip,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.SaveLoadAllManifests.Roundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_SaveLoadAllManifests_Roundtrip::RunTest(const FString& Parameters)
{
	FString TempDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ExtRegRoundtripTest"));
	IFileManager::Get().MakeDirectory(*TempDir, true);

	// Save
	FSGDTASerializerExtenderRegistry SaveRegistry;
	FName FrameworkA(TEXT("FrameworkA"));
	FName FrameworkB(TEXT("FrameworkB"));

	FSGDTAClassId Id1 = FSGDTAClassId::NewGeneratedId();
	FSGDTAClassId Id2 = FSGDTAClassId::NewGeneratedId();

	TSharedPtr<FSGDTAExtenderManifest> ManifestA = SaveRegistry.GetOrCreateManifest(FrameworkA);
	ManifestA->AddExtender(Id1, TSoftClassPtr<UObject>(USGDTASerializerExtenderBase::StaticClass()));

	TSharedPtr<FSGDTAExtenderManifest> ManifestB = SaveRegistry.GetOrCreateManifest(FrameworkB);
	ManifestB->AddExtender(Id2, TSoftClassPtr<UObject>(UObject::StaticClass()));

	bool bSaved = SaveRegistry.SaveAllManifests(TempDir);
	TestTrue(TEXT("SaveAllManifests should succeed"), bSaved);

	// Load into a new registry
	FSGDTASerializerExtenderRegistry LoadRegistry;
	bool bLoaded = LoadRegistry.LoadAllManifests(TempDir);
	TestTrue(TEXT("LoadAllManifests should succeed"), bLoaded);

	TestEqual(TEXT("Should have loaded 2 manifests"), LoadRegistry.NumManifests(), 2);

	TSharedPtr<FSGDTAExtenderManifest> LoadedA = LoadRegistry.GetManifest(FrameworkA);
	TestNotNull(TEXT("FrameworkA manifest should exist"), LoadedA.Get());
	if (LoadedA.IsValid())
	{
		TestNotNull(TEXT("Id1 should be found in FrameworkA"), LoadedA->FindByExtenderId(Id1));
	}

	TSharedPtr<FSGDTAExtenderManifest> LoadedB = LoadRegistry.GetManifest(FrameworkB);
	TestNotNull(TEXT("FrameworkB manifest should exist"), LoadedB.Get());
	if (LoadedB.IsValid())
	{
		TestNotNull(TEXT("Id2 should be found in FrameworkB"), LoadedB->FindByExtenderId(Id2));
	}

	// Clean up
	IFileManager::Get().DeleteDirectory(*TempDir, false, true);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_LoadAllManifests_InvalidSchema,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.LoadAllManifests.InvalidSchema",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_LoadAllManifests_InvalidSchema::RunTest(const FString& Parameters)
{
	// Expect the error logs from schema validation failure and registry load failure
	AddExpectedError(TEXT("Invalid schema in manifest"), EAutomationExpectedErrorFlags::Contains, 1);
	AddExpectedError(TEXT("Failed to load manifest from"), EAutomationExpectedErrorFlags::Contains, 1);

	FString TempDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ExtRegInvalidSchemaTest"));
	IFileManager::Get().MakeDirectory(*TempDir, true);

	// Write invalid schema JSON with the expected naming convention
	FString BadJson = TEXT("{\"schema\": \"wrong_schema\", \"version\": 1, \"extenders\": []}");
	FString FilePath = FPaths::Combine(TempDir, TEXT("DTA_Bad_extenders.dta.json"));
	FFileHelper::SaveStringToFile(BadJson, *FilePath);

	FSGDTASerializerExtenderRegistry Registry;
	bool bLoaded = Registry.LoadAllManifests(TempDir);
	TestFalse(TEXT("LoadAllManifests should fail when all files have invalid schema"), bLoaded);

	// Clean up
	IFileManager::Get().DeleteDirectory(*TempDir, false, true);

	return true;
}

// --- FSGDTAClassId template integration tests (using manifest directly) ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetClass_ResolvesViaManifest,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetClass.ResolvesViaManifest",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetClass_ResolvesViaManifest::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTAExtenderManifest Manifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId Id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> ClassPtr(USGDTASerializerExtenderBase::StaticClass());

	Manifest.AddExtender(Id, ClassPtr);

	// Use the template GetClass with FSGDTAExtenderManifest
	TSoftClassPtr<UObject> Resolved = Id.GetClass<FSGDTAExtenderManifest>(Manifest);
	TestFalse(TEXT("GetClass<Manifest> should return non-null for registered ID"), Resolved.IsNull());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetClass_InvalidIdReturnsEmpty,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetClass.InvalidIdReturnsEmpty",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetClass_InvalidIdReturnsEmpty::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTAExtenderManifest Manifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId InvalidId;

	TSoftClassPtr<UObject> Resolved = InvalidId.GetClass<FSGDTAExtenderManifest>(Manifest);
	TestTrue(TEXT("GetClass with invalid ID should return empty"), Resolved.IsNull());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetClass_TypedCast,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetClass.TypedCast",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetClass_TypedCast::RunTest(const FString& Parameters)
{
	using namespace SGDTAExtenderRegistryTestHelpers;

	FSGDTAExtenderManifest Manifest(TEST_FRAMEWORK_KEY);
	FSGDTAClassId Id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> ClassPtr(USGDTASerializerExtenderBase::StaticClass());

	Manifest.AddExtender(Id, ClassPtr);

	// Use the typed template GetClass
	TSoftClassPtr<USGDTASerializerExtenderBase> Resolved =
		Id.GetClassTyped<USGDTASerializerExtenderBase, FSGDTAExtenderManifest>(Manifest);
	TestFalse(TEXT("Typed GetClass should return non-null for registered ID"), Resolved.IsNull());

	return true;
}

// --- Multi-manifest registry tests ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_MultiManifest_IsolatedFrameworks,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.MultiManifest.IsolatedFrameworks",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_MultiManifest_IsolatedFrameworks::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry Registry;
	FName KeyA(TEXT("FrameworkA"));
	FName KeyB(TEXT("FrameworkB"));

	FSGDTAClassId Id = FSGDTAClassId::NewGeneratedId();
	TSoftClassPtr<UObject> ClassPtr(USGDTASerializerExtenderBase::StaticClass());

	Registry.GetOrCreateManifest(KeyA)->AddExtender(Id, ClassPtr);

	// The same ID should not be found in a different framework's manifest
	TSharedPtr<FSGDTAExtenderManifest> ManifestB = Registry.GetOrCreateManifest(KeyB);
	TestNull(TEXT("ID added to FrameworkA should not exist in FrameworkB"),
		ManifestB->FindByExtenderId(Id));

	TestEqual(TEXT("Registry should have 2 manifests"), Registry.NumManifests(), 2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetOrCreateManifest_CreatesNewForUnknownKey,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetOrCreateManifest.CreatesNewForUnknownKey",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetOrCreateManifest_CreatesNewForUnknownKey::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry Registry;
	FName newKey(TEXT("BrandNewFramework"));

	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetOrCreateManifest(newKey);
	TestNotNull(TEXT("GetOrCreateManifest should return a valid pointer"), Manifest.Get());
	TestEqual(TEXT("Registry should have 1 manifest"), Registry.NumManifests(), 1);

	if (Manifest.IsValid())
	{
		TestEqual(TEXT("Manifest should have the correct framework key"), Manifest->GetFrameworkKey(), newKey);
		TestEqual(TEXT("New manifest should be empty"), Manifest->Num(), 0);
	}

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_GetManifest_ReturnsNullForUnknownKey,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.GetManifest.ReturnsNullForUnknownKey",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_GetManifest_ReturnsNullForUnknownKey::RunTest(const FString& Parameters)
{
	FSGDTASerializerExtenderRegistry Registry;

	TSharedPtr<FSGDTAExtenderManifest> Manifest = Registry.GetManifest(FName(TEXT("NonExistentKey")));
	TestNull(TEXT("GetManifest should return nullptr for unknown key"), Manifest.Get());
	TestEqual(TEXT("Registry should have 0 manifests"), Registry.NumManifests(), 0);

	return true;
}

// --- Binary serialization tests ---

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_BakeLoadBinary_MultiManifestRoundtrip,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.BakeLoadBinary.MultiManifestRoundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_BakeLoadBinary_MultiManifestRoundtrip::RunTest(const FString& Parameters)
{
	FString TempDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ExtRegBinaryRoundtripTest"));
	IFileManager::Get().MakeDirectory(*TempDir, true);

	FName keyA(TEXT("FrameworkAlpha"));
	FName keyB(TEXT("FrameworkBeta"));
	FSGDTAClassId idA = FSGDTAClassId::NewGeneratedId();
	FSGDTAClassId idB = FSGDTAClassId::NewGeneratedId();

	// Save
	{
		FSGDTASerializerExtenderRegistry saveRegistry;
		saveRegistry.GetOrCreateManifest(keyA)->AddExtender(idA,
			TSoftClassPtr<UObject>(USGDTASerializerExtenderBase::StaticClass()));
		saveRegistry.GetOrCreateManifest(keyB)->AddExtender(idB,
			TSoftClassPtr<UObject>(UObject::StaticClass()));

		bool bBaked = saveRegistry.BakeAllManifests(TempDir);
		TestTrue(TEXT("BakeAllManifests should succeed"), bBaked);
	}

	// Load into new registry
	{
		FSGDTASerializerExtenderRegistry loadRegistry;
		bool bLoaded = loadRegistry.LoadAllManifestsBinary(TempDir);
		TestTrue(TEXT("LoadAllManifestsBinary should succeed"), bLoaded);
		TestEqual(TEXT("Should have loaded 2 manifests"), loadRegistry.NumManifests(), 2);

		TSharedPtr<FSGDTAExtenderManifest> loadedA = loadRegistry.GetManifest(keyA);
		TestNotNull(TEXT("FrameworkAlpha manifest should exist"), loadedA.Get());
		if (loadedA.IsValid())
		{
			const FSGDTASerializerExtenderRegistryEntry* entryA = loadedA->FindByExtenderId(idA);
			TestNotNull(TEXT("idA should be found in FrameworkAlpha"), entryA);
			if (entryA)
			{
				TestEqual(TEXT("ClassName should match for idA"),
					entryA->ClassName, TEXT("SGDTASerializerExtenderBase"));
			}
		}

		TSharedPtr<FSGDTAExtenderManifest> loadedB = loadRegistry.GetManifest(keyB);
		TestNotNull(TEXT("FrameworkBeta manifest should exist"), loadedB.Get());
		if (loadedB.IsValid())
		{
			const FSGDTASerializerExtenderRegistryEntry* entryB = loadedB->FindByExtenderId(idB);
			TestNotNull(TEXT("idB should be found in FrameworkBeta"), entryB);
			if (entryB)
			{
				TestEqual(TEXT("ClassName should match for idB"),
					entryB->ClassName, TEXT("Object"));
			}
		}
	}

	IFileManager::Get().DeleteDirectory(*TempDir, false, true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_BinaryManifest_SingleManifestRoundtrip,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.BinaryManifest.SingleManifestRoundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_BinaryManifest_SingleManifestRoundtrip::RunTest(const FString& Parameters)
{
	FString TempDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ExtManifestBinaryRoundtripTest"));
	IFileManager::Get().MakeDirectory(*TempDir, true);
	FString filePath = FPaths::Combine(TempDir, TEXT("test_manifest.dta.bin"));

	FSGDTAClassId id1 = FSGDTAClassId::NewGeneratedId();
	FSGDTAClassId id2 = FSGDTAClassId::NewGeneratedId();
	FName frameworkKey(TEXT("TestBinaryFramework"));

	// Save
	{
		FSGDTAExtenderManifest saveManifest(frameworkKey);
		saveManifest.AddExtender(id1, TSoftClassPtr<UObject>(USGDTASerializerExtenderBase::StaticClass()));
		saveManifest.AddExtender(id2, TSoftClassPtr<UObject>(UObject::StaticClass()));

		bool bSaved = saveManifest.SaveToBinaryFile(filePath);
		TestTrue(TEXT("SaveToBinaryFile should succeed"), bSaved);
	}

	// Load
	{
		FSGDTAExtenderManifest loadManifest(frameworkKey);
		bool bLoaded = loadManifest.LoadFromBinaryFile(filePath);
		TestTrue(TEXT("LoadFromBinaryFile should succeed"), bLoaded);
		TestEqual(TEXT("Loaded manifest should have 2 entries"), loadManifest.Num(), 2);

		const FSGDTASerializerExtenderRegistryEntry* entry1 = loadManifest.FindByExtenderId(id1);
		TestNotNull(TEXT("id1 should be found after binary round-trip"), entry1);
		if (entry1)
		{
			TestEqual(TEXT("id1 ClassName should survive round-trip"),
				entry1->ClassName, TEXT("SGDTASerializerExtenderBase"));
		}

		const FSGDTASerializerExtenderRegistryEntry* entry2 = loadManifest.FindByExtenderId(id2);
		TestNotNull(TEXT("id2 should be found after binary round-trip"), entry2);
		if (entry2)
		{
			TestEqual(TEXT("id2 ClassName should survive round-trip"),
				entry2->ClassName, TEXT("Object"));
		}
	}

	IFileManager::Get().DeleteDirectory(*TempDir, false, true);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FExtenderRegistry_BinaryHeader_InvalidMagicFailsLoad,
	"SGDynamicTextAssets.Runtime.Management.ExtenderRegistry.BinaryHeader.InvalidMagicFailsLoad",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FExtenderRegistry_BinaryHeader_InvalidMagicFailsLoad::RunTest(const FString& Parameters)
{
	AddExpectedError(TEXT("Invalid magic number"), EAutomationExpectedErrorFlags::Contains, 1);

	FString TempDir = FPaths::Combine(FPaths::ProjectSavedDir(), TEXT("ExtBinaryHeaderTest"));
	IFileManager::Get().MakeDirectory(*TempDir, true);
	FString filePath = FPaths::Combine(TempDir, TEXT("corrupted.dta.bin"));

	// Write a valid binary manifest
	{
		FSGDTAExtenderManifest manifest(FName(TEXT("HeaderTest")));
		manifest.AddExtender(FSGDTAClassId::NewGeneratedId(),
			TSoftClassPtr<UObject>(USGDTASerializerExtenderBase::StaticClass()));
		manifest.SaveToBinaryFile(filePath);
	}

	// Corrupt the magic number (first 4 bytes)
	TArray<uint8> fileData;
	FFileHelper::LoadFileToArray(fileData, *filePath);
	TestTrue(TEXT("File should have at least header size bytes"),
		fileData.Num() >= static_cast<int32>(FSGDTAExtenderManifestBinaryHeader::HEADER_SIZE));

	fileData[0] = 0xFF;
	fileData[1] = 0xFF;
	fileData[2] = 0xFF;
	fileData[3] = 0xFF;
	FFileHelper::SaveArrayToFile(fileData, *filePath);

	// Attempt to load the corrupted file
	FSGDTAExtenderManifest loadManifest(FName(TEXT("HeaderTest")));
	bool bLoaded = loadManifest.LoadFromBinaryFile(filePath);
	TestFalse(TEXT("LoadFromBinaryFile should fail with corrupted magic number"), bLoaded);

	IFileManager::Get().DeleteDirectory(*TempDir, false, true);
	return true;
}
