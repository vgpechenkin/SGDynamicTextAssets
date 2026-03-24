// Copyright Start Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Core/SGDTAClassId.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/BitWriter.h"
#include "Serialization/BitReader.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_DefaultState_IsInvalid,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.DefaultState.IsInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_DefaultState_IsInvalid::RunTest(const FString& Parameters)
{
	FSGDTAClassId defaultId;

	TestFalse(TEXT("Default-constructed ClassId should be invalid"), defaultId.IsValid());
	TestFalse(TEXT("Default-constructed underlying GUID should be invalid"), defaultId.GetGuid().IsValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_INVALID_CLASS_ID_IsInvalid,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.INVALID_CLASS_ID.IsInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_INVALID_CLASS_ID_IsInvalid::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("INVALID_CLASS_ID should be invalid"), FSGDTAClassId::INVALID_CLASS_ID.IsValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_NewGeneratedId_IsValid,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.NewGeneratedId.IsValid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_NewGeneratedId_IsValid::RunTest(const FString& Parameters)
{
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();

	TestTrue(TEXT("NewGeneratedId should return a valid ClassId"), id.IsValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_NewGeneratedId_ProducesUniqueIds,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.NewGeneratedId.ProducesUniqueIds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_NewGeneratedId_ProducesUniqueIds::RunTest(const FString& Parameters)
{
	FSGDTAClassId id1 = FSGDTAClassId::NewGeneratedId();
	FSGDTAClassId id2 = FSGDTAClassId::NewGeneratedId();

	TestTrue(TEXT("First generated ClassId should be valid"), id1.IsValid());
	TestTrue(TEXT("Second generated ClassId should be valid"), id2.IsValid());
	TestTrue(TEXT("Two generated ClassIds should be different"), id1 != id2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_FromGuid_Roundtrip,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.FromGuid.Roundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_FromGuid_Roundtrip::RunTest(const FString& Parameters)
{
	// Valid GUID
	FGuid validGuid = FGuid::NewGuid();
	FSGDTAClassId id = FSGDTAClassId::FromGuid(validGuid);

	TestTrue(TEXT("FromGuid with valid GUID should produce valid ClassId"), id.IsValid());
	TestTrue(TEXT("GetGuid should return the original GUID"), id.GetGuid() == validGuid);

	// Invalid GUID
	FGuid invalidGuid;
	FSGDTAClassId invalidId = FSGDTAClassId::FromGuid(invalidGuid);

	TestFalse(TEXT("FromGuid with invalid GUID should produce invalid ClassId"), invalidId.IsValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_FromString_ValidString,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.FromString.ValidString",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_FromString_ValidString::RunTest(const FString& Parameters)
{
	FSGDTAClassId id = FSGDTAClassId::FromString(TEXT("550E8400-E29B-41D4-A716-446655440000"));

	TestTrue(TEXT("FromString with valid GUID string should produce valid ClassId"), id.IsValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_FromString_InvalidString,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.FromString.InvalidString",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_FromString_InvalidString::RunTest(const FString& Parameters)
{
	FSGDTAClassId emptyId = FSGDTAClassId::FromString(TEXT(""));
	TestFalse(TEXT("FromString with empty string should produce invalid ClassId"), emptyId.IsValid());

	FSGDTAClassId garbageId = FSGDTAClassId::FromString(TEXT("not-a-valid-guid"));
	TestFalse(TEXT("FromString with garbage string should produce invalid ClassId"), garbageId.IsValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_ToString_Roundtrip,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.ToString.Roundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_ToString_Roundtrip::RunTest(const FString& Parameters)
{
	FSGDTAClassId original = FSGDTAClassId::NewGeneratedId();

	FString str = original.ToString();
	TestFalse(TEXT("ToString of valid ClassId should return non-empty string"), str.IsEmpty());

	FSGDTAClassId parsed = FSGDTAClassId::FromString(str);
	TestTrue(TEXT("FromString(id.ToString()) should roundtrip to the same ClassId"), parsed == original);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_ParseString_ValidAndInvalid,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.ParseString.ValidAndInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_ParseString_ValidAndInvalid::RunTest(const FString& Parameters)
{
	// Valid parse
	FSGDTAClassId id;
	bool bResult = id.ParseString(TEXT("550E8400-E29B-41D4-A716-446655440000"));
	TestTrue(TEXT("ParseString with valid GUID should return true"), bResult);
	TestTrue(TEXT("ClassId should be valid after successful ParseString"), id.IsValid());

	// Invalid parse
	FSGDTAClassId id2;
	bool bResult2 = id2.ParseString(TEXT("not-a-guid"));
	TestFalse(TEXT("ParseString with invalid string should return false"), bResult2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_Equality_SameGuid,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.Equality.SameGuid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_Equality_SameGuid::RunTest(const FString& Parameters)
{
	FGuid guid = FGuid::NewGuid();
	FSGDTAClassId id1 = FSGDTAClassId::FromGuid(guid);
	FSGDTAClassId id2 = FSGDTAClassId::FromGuid(guid);

	TestTrue(TEXT("Two ClassIds from the same GUID should be equal"), id1 == id2);
	TestFalse(TEXT("Two ClassIds from the same GUID should not be unequal"), id1 != id2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_Equality_DifferentGuid,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.Equality.DifferentGuid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_Equality_DifferentGuid::RunTest(const FString& Parameters)
{
	FSGDTAClassId id1 = FSGDTAClassId::NewGeneratedId();
	FSGDTAClassId id2 = FSGDTAClassId::NewGeneratedId();

	TestFalse(TEXT("Two different ClassIds should not be equal"), id1 == id2);
	TestTrue(TEXT("Two different ClassIds should be unequal"), id1 != id2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_Equality_TwoInvalidAreEqual,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.Equality.TwoInvalidAreEqual",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_Equality_TwoInvalidAreEqual::RunTest(const FString& Parameters)
{
	FSGDTAClassId id1;
	FSGDTAClassId id2;

	TestTrue(TEXT("Two default-constructed (invalid) ClassIds should be equal"), id1 == id2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_Equality_ClassIdVsRawGuid,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.Equality.ClassIdVsRawGuid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_Equality_ClassIdVsRawGuid::RunTest(const FString& Parameters)
{
	FGuid guid = FGuid::NewGuid();
	FSGDTAClassId classId = FSGDTAClassId::FromGuid(guid);

	// operator==(const FGuid&)
	TestTrue(TEXT("ClassId should equal the FGuid it was constructed from"), classId == guid);
	TestFalse(TEXT("ClassId should not be unequal to the FGuid it was constructed from"), classId != guid);

	// Different GUID
	FGuid otherGuid = FGuid::NewGuid();
	TestFalse(TEXT("ClassId should not equal a different FGuid"), classId == otherGuid);
	TestTrue(TEXT("ClassId should be unequal to a different FGuid"), classId != otherGuid);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_GetTypeHash_ConsistentForSameId,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.GetTypeHash.ConsistentForSameId",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_GetTypeHash_ConsistentForSameId::RunTest(const FString& Parameters)
{
	FGuid guid = FGuid::NewGuid();
	FSGDTAClassId id1 = FSGDTAClassId::FromGuid(guid);
	FSGDTAClassId id2 = FSGDTAClassId::FromGuid(guid);

	uint32 hash1 = GetTypeHash(id1);
	uint32 hash2 = GetTypeHash(id2);

	TestEqual(TEXT("Hash of two ClassIds with same GUID should be equal"), hash1, hash2);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_GetTypeHash_DifferentForDifferentIds,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.GetTypeHash.DifferentForDifferentIds",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_GetTypeHash_DifferentForDifferentIds::RunTest(const FString& Parameters)
{
	// Generate several pairs to reduce false-positive probability
	int32 differentCount = 0;
	constexpr int32 NUM_TRIALS = 10;

	for (int32 i = 0; i < NUM_TRIALS; ++i)
	{
		FSGDTAClassId id1 = FSGDTAClassId::NewGeneratedId();
		FSGDTAClassId id2 = FSGDTAClassId::NewGeneratedId();

		if (GetTypeHash(id1) != GetTypeHash(id2))
		{
			++differentCount;
		}
	}

	// With 32-bit hashes, collision probability per pair is ~1/2^32.
	// Expect all or nearly all to differ.
	TestTrue(TEXT("Hashes of different ClassIds should usually differ (probabilistic)"), differentCount >= NUM_TRIALS - 1);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_Serialize_BinaryRoundtrip,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.Serialize.BinaryRoundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_Serialize_BinaryRoundtrip::RunTest(const FString& Parameters)
{
	FSGDTAClassId original = FSGDTAClassId::NewGeneratedId();

	// Write
	TArray<uint8> buffer;
	FMemoryWriter writer(buffer);
	original.Serialize(writer);

	TestTrue(TEXT("Serialized buffer should not be empty"), buffer.Num() > 0);

	// Read
	FSGDTAClassId deserialized;
	FMemoryReader reader(buffer);
	deserialized.Serialize(reader);

	TestTrue(TEXT("Deserialized ClassId should match original"), deserialized == original);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_NetSerialize_Roundtrip,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.NetSerialize.Roundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_NetSerialize_Roundtrip::RunTest(const FString& Parameters)
{
	FSGDTAClassId original = FSGDTAClassId::NewGeneratedId();

	// Write - FGuid is 16 bytes = 128 bits, allocate enough
	TArray<uint8> buffer;
	buffer.SetNumZeroed(256);
	FBitWriter writer(256 * 8);

	bool bWriteSuccess = false;
	original.NetSerialize(writer, nullptr, bWriteSuccess);
	TestTrue(TEXT("NetSerialize write should succeed"), bWriteSuccess);

	// Read
	FBitReader reader(writer.GetData(), writer.GetNumBits());
	FSGDTAClassId deserialized;
	bool bReadSuccess = false;
	deserialized.NetSerialize(reader, nullptr, bReadSuccess);
	TestTrue(TEXT("NetSerialize read should succeed"), bReadSuccess);

	TestTrue(TEXT("Net-deserialized ClassId should match original"), deserialized == original);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_ExportImportTextItem_Roundtrip,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.ExportImportTextItem.Roundtrip",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_ExportImportTextItem_Roundtrip::RunTest(const FString& Parameters)
{
	FSGDTAClassId original = FSGDTAClassId::NewGeneratedId();

	// Export
	FString exported;
	FSGDTAClassId defaultValue;
	bool bExported = original.ExportTextItem(exported, defaultValue, nullptr, 0, nullptr);

	TestTrue(TEXT("ExportTextItem should succeed"), bExported);
	TestFalse(TEXT("Exported string should not be empty"), exported.IsEmpty());

	// Import
	FSGDTAClassId imported;
	const TCHAR* importBuffer = *exported;
	bool bImported = imported.ImportTextItem(importBuffer, 0, nullptr, nullptr);

	TestTrue(TEXT("ImportTextItem should succeed"), bImported);
	TestTrue(TEXT("Imported ClassId should match original"), imported == original);

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_Invalidate_BecomesInvalid,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.Invalidate.BecomesInvalid",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_Invalidate_BecomesInvalid::RunTest(const FString& Parameters)
{
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();
	TestTrue(TEXT("ClassId should be valid before invalidation"), id.IsValid());

	id.Invalidate();

	TestFalse(TEXT("ClassId should be invalid after Invalidate()"), id.IsValid());

	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(
	FDTAClassId_GenerateNewId_ChangesValue,
	"SGDynamicTextAssets.Runtime.Core.DTAClassId.GenerateNewId.ChangesValue",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter)

bool FDTAClassId_GenerateNewId_ChangesValue::RunTest(const FString& Parameters)
{
	FSGDTAClassId id = FSGDTAClassId::NewGeneratedId();
	FGuid originalGuid = id.GetGuid();

	TestTrue(TEXT("ClassId should be valid before GenerateNewId"), id.IsValid());

	id.GenerateNewId();

	TestTrue(TEXT("ClassId should still be valid after GenerateNewId"), id.IsValid());
	TestTrue(TEXT("GenerateNewId should change the underlying GUID"), id.GetGuid() != originalGuid);

	return true;
}
