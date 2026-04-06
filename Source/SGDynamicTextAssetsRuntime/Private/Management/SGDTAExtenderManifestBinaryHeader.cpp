// Copyright Start Games, Inc. All Rights Reserved.

#include "Management/SGDTAExtenderManifestBinaryHeader.h"

// Validate the struct is properly defined.
static_assert(sizeof(FSGDTAExtenderManifestBinaryHeader::MAGIC_NUMBER) == 4,
	"Magic number must be exactly 4 bytes");
static_assert(sizeof(FSGDTAExtenderManifestBinaryHeader) == FSGDTAExtenderManifestBinaryHeader::HEADER_SIZE,
	"FSGDTAExtenderManifestBinaryHeader size mismatch - check member alignment");

FSGDTAExtenderManifestBinaryHeader::FSGDTAExtenderManifestBinaryHeader()
{
	FMemory::Memcpy(MagicNumber, MAGIC_NUMBER, sizeof(MagicNumber));
}

bool FSGDTAExtenderManifestBinaryHeader::IsValid() const
{
	return FMemory::Memcmp(MagicNumber, MAGIC_NUMBER, sizeof(MagicNumber)) == 0;
}

bool FSGDTAExtenderManifestBinaryHeader::HasContentHash() const
{
	for (uint32 index = 0; index < CONTENT_HASH_SIZE; ++index)
	{
		if (ContentHash[index] != 0)
		{
			return true;
		}
	}
	return false;
}

FString FSGDTAExtenderManifestBinaryHeader::GetMagicNumberString() const
{
	FString result = TEXT("0x");
	for (int32 index = 0; index < sizeof(MagicNumber); ++index)
	{
		result += FString::Printf(TEXT("%02X"), MagicNumber[index]);
	}
	return result;
}

FString FSGDTAExtenderManifestBinaryHeader::GetExpectedMagicNumberString()
{
	FString result = TEXT("0x");
	static constexpr uint8 size = sizeof(MAGIC_NUMBER);
	for (int32 index = 0; index < size; ++index)
	{
		result += FString::Printf(TEXT("%02X"), MAGIC_NUMBER[index]);
	}
	return result;
}

void FSGDTAExtenderManifestBinaryHeader::Serialize(FArchive& Ar)
{
	Ar.Serialize(MagicNumber, sizeof(MagicNumber));
	Ar << Version;
	Ar << EntryCount;
	Ar << StringTableOffset;
	Ar.Serialize(Reserved, sizeof(Reserved));
	Ar.Serialize(ContentHash, sizeof(ContentHash));
}
