// Copyright Start Games, Inc. All Rights Reserved.

#include "Core/SGSerializerFormat.h"

#include "Management/SGDynamicTextAssetFileManager.h"
#include "Serialization/SGDynamicTextAssetSerializer.h"

const FSGSerializerFormat FSGSerializerFormat::INVALID;

FSGSerializerFormat::FSGSerializerFormat(const uint32& InTypeId)
	: TypeId(InTypeId)
{
}

bool FSGSerializerFormat::IsValid() const
{
	return TypeId != INVALID.TypeId;
}

uint32 FSGSerializerFormat::GetTypeId() const
{
	return TypeId;
}

void FSGSerializerFormat::SetTypeId(const uint32& InTypeId)
{
	TypeId = InTypeId;
}

FString FSGSerializerFormat::ToString() const
{
	return FString::FromInt(static_cast<int32>(TypeId));
}

FSGSerializerFormat FSGSerializerFormat::FromTypeId(uint32 InTypeId)
{
	return FSGSerializerFormat(InTypeId);
}

FSGSerializerFormat FSGSerializerFormat::FromExtension(const FString& Extension)
{
	return FSGDynamicTextAssetFileManager::GetFormatForExtension(Extension);
}

FText FSGSerializerFormat::GetFormatName() const
{
	if (TSharedPtr<ISGDynamicTextAssetSerializer> serializer = FSGDynamicTextAssetFileManager::FindSerializerForFormat(*this))
	{
		return serializer->GetFormatName();
	}
	return INVTEXT("Invalid");
}

FString FSGSerializerFormat::GetFileExtension() const
{
	if (TSharedPtr<ISGDynamicTextAssetSerializer> serializer = FSGDynamicTextAssetFileManager::FindSerializerForFormat(*this))
	{
		return serializer->GetFileExtension();
	}
	return FString();
}

FText FSGSerializerFormat::GetFormatDescription() const
{
	if (TSharedPtr<ISGDynamicTextAssetSerializer> serializer = FSGDynamicTextAssetFileManager::FindSerializerForFormat(*this))
	{
		return serializer->GetFormatDescription();
	}
	return INVTEXT("Invalid");
}

TSharedPtr<ISGDynamicTextAssetSerializer> FSGSerializerFormat::GetSerializer() const
{
	return FSGDynamicTextAssetFileManager::FindSerializerForFormat(*this);
}

void FSGSerializerFormat::operator=(const uint32& Other)
{
	TypeId = Other;
}

bool FSGSerializerFormat::operator==(const FSGSerializerFormat& Other) const
{
	return TypeId == Other.TypeId;
}

bool FSGSerializerFormat::operator!=(const FSGSerializerFormat& Other) const
{
	return TypeId != Other.TypeId;
}

bool FSGSerializerFormat::operator==(const uint32& Other) const
{
	return TypeId == Other;
}

bool FSGSerializerFormat::operator!=(const uint32& Other) const
{
	return TypeId != Other;
}

bool FSGSerializerFormat::Serialize(FArchive& Ar)
{
	Ar << TypeId;
	return true;
}

bool FSGSerializerFormat::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << TypeId;
	bOutSuccess = true;
	return true;
}

bool FSGSerializerFormat::ExportTextItem(FString& ValueStr, const FSGSerializerFormat& DefaultValue,
	UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const
{
	ValueStr += ToString();
	return true;
}

bool FSGSerializerFormat::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags,
	UObject* Parent, FOutputDevice* ErrorText)
{
	FString digitString;
	const TCHAR* start = Buffer;

	// Consume decimal digit characters
	while (*Buffer != TEXT('\0') && FChar::IsDigit(*Buffer))
	{
		digitString.AppendChar(*Buffer);
		++Buffer;
	}

	if (digitString.IsEmpty())
	{
		// Restore position on failure
		Buffer = start;
		return false;
	}

	TypeId = static_cast<uint32>(FCString::Atoi(*digitString));
	return true;
}
