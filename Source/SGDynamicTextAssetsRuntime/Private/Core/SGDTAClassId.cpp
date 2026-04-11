// Copyright Start Games, Inc. All Rights Reserved.

#include "Core/SGDTAClassId.h"

const FSGDTAClassId FSGDTAClassId::INVALID_CLASS_ID;

FSGDTAClassId::FSGDTAClassId(const FGuid& InGuid)
	: Guid(InGuid)
{

}

FSGDTAClassId::FSGDTAClassId(FGuid&& InGuid)
	: Guid(InGuid)
{
}

bool FSGDTAClassId::IsValid() const
{
	return Guid.IsValid();
}

const FGuid& FSGDTAClassId::GetGuid() const
{
	return Guid;
}

void FSGDTAClassId::SetGuid(const FGuid& InGuid)
{
#if WITH_EDITOR
	const FGuid prevGuid = Guid;
#endif
	Guid = InGuid;
#if WITH_EDITOR
	OnGuidChanged_Editor.Broadcast(prevGuid, Guid);
#endif
}

bool FSGDTAClassId::ParseString(const FString& GuidString)
{
	FGuid guid;
	bool result = FGuid::Parse(GuidString, guid);
	SetGuid(guid);
	return result;
}

void FSGDTAClassId::Invalidate()
{
#if WITH_EDITOR
	const FGuid prevGuid = Guid;
#endif
	Guid.Invalidate();
#if WITH_EDITOR
	OnGuidChanged_Editor.Broadcast(prevGuid, Guid);
#endif
}

FString FSGDTAClassId::ToString() const
{
	return Guid.ToString(EGuidFormats::DigitsWithHyphens);
}

void FSGDTAClassId::GenerateNewId()
{
	SetGuid(FGuid::NewGuid());
}

FSGDTAClassId FSGDTAClassId::NewGeneratedId()
{
	return FSGDTAClassId(FGuid::NewGuid());
}

FSGDTAClassId FSGDTAClassId::FromGuid(const FGuid& InGuid)
{
	return FSGDTAClassId(InGuid);
}

FSGDTAClassId FSGDTAClassId::FromString(const FString& InString)
{
	FGuid parsedGuid;
	if (FGuid::Parse(InString, parsedGuid))
	{
		return FSGDTAClassId(parsedGuid);
	}
	return FSGDTAClassId();
}

void FSGDTAClassId::operator=(const FGuid& Other)
{
	SetGuid(Other);
}

bool FSGDTAClassId::operator==(const FGuid& Other) const
{
	return Guid == Other;
}

bool FSGDTAClassId::operator!=(const FGuid& Other) const
{
	return Guid != Other;
}

bool FSGDTAClassId::operator==(const FSGDTAClassId& Other) const
{
	return Guid == Other.Guid;
}

bool FSGDTAClassId::operator!=(const FSGDTAClassId& Other) const
{
	return Guid != Other.Guid;
}

bool FSGDTAClassId::Serialize(FArchive& Ar)
{
	Ar << Guid;
	return true;
}

bool FSGDTAClassId::NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess)
{
	Ar << Guid;
	bOutSuccess = true;
	return true;
}

bool FSGDTAClassId::ExportTextItem(FString& ValueStr, const FSGDTAClassId& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const
{
	ValueStr += ToString();
	return true;
}

bool FSGDTAClassId::ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText)
{
	// Read up to 36 characters (GUID with hyphens: 8-4-4-4-12)
	FString guidString;
	const TCHAR* start = Buffer;

	// Consume characters that are valid hex digits or hyphens
	while (*Buffer != TEXT('\0') && ((FChar::IsHexDigit(*Buffer) || *Buffer == TEXT('-'))))
	{
		guidString.AppendChar(*Buffer);
		++Buffer;
	}

	if (guidString.IsEmpty())
	{
		// Restore position on failure
		Buffer = start;
		return false;
	}

	if (ParseString(guidString))
	{
		return true;
	}

	// Restore position on failure
	Buffer = start;
	return false;
}
