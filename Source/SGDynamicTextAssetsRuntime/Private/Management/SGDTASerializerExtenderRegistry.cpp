// Copyright Start Games, Inc. All Rights Reserved.

#include "Management/SGDTASerializerExtenderRegistry.h"

#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "SGDynamicTextAssetLogs.h"

const FString FSGDTASerializerExtenderRegistry::KEY_SCHEMA = TEXT("schema");
const FString FSGDTASerializerExtenderRegistry::VALUE_SCHEMA = TEXT("dta_extender_registry");
const FString FSGDTASerializerExtenderRegistry::KEY_VERSION = TEXT("version");
const FString FSGDTASerializerExtenderRegistry::KEY_EXTENDERS = TEXT("extenders");
const FString FSGDTASerializerExtenderRegistry::KEY_EXTENDER_ID = TEXT("extenderId");
const FString FSGDTASerializerExtenderRegistry::KEY_CLASS_NAME = TEXT("className");
const FString FSGDTASerializerExtenderRegistry::KEY_CLASS_PATH = TEXT("classPath");

FSGDTASerializerExtenderRegistryEntry::FSGDTASerializerExtenderRegistryEntry(
	const FSGDTAClassId& InExtenderId,
	const TSoftClassPtr<UObject>& InClass)
	: ExtenderId(InExtenderId)
	, Class(InClass)
{
	// Extract class name from the soft class path for fast lookup
	FString classPath = InClass.ToString();
	if (!classPath.IsEmpty())
	{
		int32 lastDot = INDEX_NONE;
		if (classPath.FindLastChar(TEXT('.'), lastDot))
		{
			ClassName = classPath.Mid(lastDot + 1);
		}
		else
		{
			ClassName = classPath;
		}
	}
}

bool FSGDTASerializerExtenderRegistry::LoadFromFile(const FString& FilePath)
{
	Clear();

	FString jsonString;
	if (!FFileHelper::LoadFileToString(jsonString, *FilePath))
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
			TEXT("FSGDTASerializerExtenderRegistry: Registry not found at '%s'"), *FilePath);
		return false;
	}

	TSharedPtr<FJsonObject> rootObject;
	TSharedRef<TJsonReader<>> reader = TJsonReaderFactory<>::Create(jsonString);

	if (!FJsonSerializer::Deserialize(reader, rootObject) || !rootObject.IsValid())
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Error,
			TEXT("FSGDTASerializerExtenderRegistry: Failed to parse registry JSON from '%s'"), *FilePath);
		return false;
	}

	// Validate schema
	FString schema;
	if (!rootObject->TryGetStringField(KEY_SCHEMA, schema) || schema != VALUE_SCHEMA)
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Error,
			TEXT("FSGDTASerializerExtenderRegistry: Invalid schema in registry '%s'"), *FilePath);
		return false;
	}

	// Validate version
	int32 version = 0;
	if (!rootObject->TryGetNumberField(KEY_VERSION, version) || version > REGISTRY_VERSION)
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Error,
			TEXT("FSGDTASerializerExtenderRegistry: Unsupported registry version %d (max supported: %d)"),
			version, REGISTRY_VERSION);
		return false;
	}

	// Parse extenders array
	const TArray<TSharedPtr<FJsonValue>>* extendersArray = nullptr;
	if (!rootObject->TryGetArrayField(KEY_EXTENDERS, extendersArray) || extendersArray == nullptr)
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Error,
			TEXT("FSGDTASerializerExtenderRegistry: No '%s' array in registry '%s'"), *KEY_EXTENDERS, *FilePath);
		return false;
	}

	Extenders.Reserve(extendersArray->Num());

	for (const TSharedPtr<FJsonValue>& extenderValue : *extendersArray)
	{
		const TSharedPtr<FJsonObject>* extenderObjectPtr = nullptr;
		if (!extenderValue.IsValid() || !extenderValue->TryGetObject(extenderObjectPtr) || extenderObjectPtr == nullptr)
		{
			UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
				TEXT("FSGDTASerializerExtenderRegistry: Skipping invalid extender entry in registry"));
			continue;
		}

		FSGDTASerializerExtenderRegistryEntry entry;
		if (ParseExtenderEntry(*extenderObjectPtr, entry))
		{
			Extenders.Add(MoveTemp(entry));
		}
	}

	RebuildIndices();
	bIsDirty = false;

	UE_LOG(LogSGDynamicTextAssetsRuntime, Log,
		TEXT("FSGDTASerializerExtenderRegistry: Loaded registry with %d extenders from '%s'"), Extenders.Num(), *FilePath);
	return true;
}

bool FSGDTASerializerExtenderRegistry::SaveToFile(const FString& FilePath) const
{
	TSharedRef<FJsonObject> rootObject = MakeShared<FJsonObject>();
	rootObject->SetStringField(KEY_SCHEMA, VALUE_SCHEMA);
	rootObject->SetNumberField(KEY_VERSION, REGISTRY_VERSION);

	TArray<TSharedPtr<FJsonValue>> extendersArray;
	extendersArray.Reserve(Extenders.Num());

	for (const FSGDTASerializerExtenderRegistryEntry& entry : Extenders)
	{
		TSharedRef<FJsonObject> extenderObject = MakeShared<FJsonObject>();
		extenderObject->SetStringField(KEY_EXTENDER_ID, entry.ExtenderId.ToString());
		extenderObject->SetStringField(KEY_CLASS_NAME, entry.ClassName);
		extenderObject->SetStringField(KEY_CLASS_PATH, entry.Class.ToString());
		extendersArray.Add(MakeShared<FJsonValueObject>(extenderObject));
	}

	rootObject->SetArrayField(KEY_EXTENDERS, extendersArray);

	FString outputString;
	TSharedRef<TJsonWriter<>> writer = TJsonWriterFactory<>::Create(&outputString);

	if (!FJsonSerializer::Serialize(rootObject, writer))
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Error,
			TEXT("FSGDTASerializerExtenderRegistry: Failed to serialize registry JSON"));
		return false;
	}

	if (!FFileHelper::SaveStringToFile(outputString, *FilePath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Error,
			TEXT("FSGDTASerializerExtenderRegistry: Failed to write registry to '%s'"), *FilePath);
		return false;
	}

	UE_LOG(LogSGDynamicTextAssetsRuntime, Log,
		TEXT("FSGDTASerializerExtenderRegistry: Saved registry with %d extenders to '%s'"), Extenders.Num(), *FilePath);
	return true;
}

const FSGDTASerializerExtenderRegistryEntry* FSGDTASerializerExtenderRegistry::FindByExtenderId(
	const FSGDTAClassId& ExtenderId) const
{
	const int32* indexPtr = ExtenderIdIndex.Find(ExtenderId);
	if (indexPtr == nullptr)
	{
		return nullptr;
	}
	return &Extenders[*indexPtr];
}

const FSGDTASerializerExtenderRegistryEntry* FSGDTASerializerExtenderRegistry::FindByClassName(
	const FString& ClassName) const
{
	const int32* indexPtr = ClassNameIndex.Find(ClassName);
	if (indexPtr == nullptr)
	{
		return nullptr;
	}
	return &Extenders[*indexPtr];
}

void FSGDTASerializerExtenderRegistry::AddExtender(const FSGDTAClassId& ExtenderId,
	const TSoftClassPtr<UObject>& InClass)
{
	if (!ExtenderId.IsValid())
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
			TEXT("FSGDTASerializerExtenderRegistry::AddExtender: Ignoring entry with invalid ExtenderId"));
		return;
	}

	// Check if extender already exists - replace it
	const int32* existingIndex = ExtenderIdIndex.Find(ExtenderId);
	if (existingIndex != nullptr)
	{
		FSGDTASerializerExtenderRegistryEntry& existing = Extenders[*existingIndex];

		// Remove old class name index entry
		ClassNameIndex.Remove(existing.ClassName);

		// Update in place
		existing = FSGDTASerializerExtenderRegistryEntry(ExtenderId, InClass);

		// Re-add class name index
		ClassNameIndex.Add(existing.ClassName, *existingIndex);
	}
	else
	{
		int32 index = Extenders.Emplace(ExtenderId, InClass);
		ExtenderIdIndex.Add(ExtenderId, index);
		ClassNameIndex.Add(Extenders[index].ClassName, index);
	}

	bIsDirty = true;
}

bool FSGDTASerializerExtenderRegistry::RemoveExtender(const FSGDTAClassId& ExtenderId)
{
	const int32* indexPtr = ExtenderIdIndex.Find(ExtenderId);
	if (indexPtr == nullptr)
	{
		return false;
	}

	int32 removeIndex = *indexPtr;
	const FString& className = Extenders[removeIndex].ClassName;

	// Remove from indices
	ExtenderIdIndex.Remove(ExtenderId);
	ClassNameIndex.Remove(className);

	// Swap-remove from array and fix up indices
	Extenders.RemoveAtSwap(removeIndex);

	// If we swapped the last element into this position, update its index entries
	if (removeIndex < Extenders.Num())
	{
		const FSGDTASerializerExtenderRegistryEntry& swapped = Extenders[removeIndex];
		ExtenderIdIndex.Add(swapped.ExtenderId, removeIndex);
		ClassNameIndex.Add(swapped.ClassName, removeIndex);
	}

	bIsDirty = true;
	return true;
}

const TArray<FSGDTASerializerExtenderRegistryEntry>& FSGDTASerializerExtenderRegistry::GetAllExtenders() const
{
	return Extenders;
}

void FSGDTASerializerExtenderRegistry::GetAllEffectiveExtenders(
	TArray<FSGDTASerializerExtenderRegistryEntry>& OutEntries) const
{
	OutEntries.Reset();

	if (!HasServerOverrides())
	{
		OutEntries = Extenders;
		return;
	}

	OutEntries.Reserve(Extenders.Num() + ServerOverlayEntries.Num());
	TSet<FSGDTAClassId> processedIds;

	// Process local entries with server overlay consideration
	for (const FSGDTASerializerExtenderRegistryEntry& entry : Extenders)
	{
		if (const FSGDTASerializerExtenderRegistryEntry* effective = GetEffectiveEntry(entry.ExtenderId))
		{
			OutEntries.Add(*effective);
		}
		processedIds.Add(entry.ExtenderId);
	}

	// Add server-only entries (new extenders not in local registry)
	for (const TPair<FSGDTAClassId, FSGDTASerializerExtenderRegistryEntry>& pair : ServerOverlayEntries)
	{
		if (!processedIds.Contains(pair.Key) && !pair.Value.ClassName.IsEmpty())
		{
			OutEntries.Add(pair.Value);
		}
	}
}

TSoftClassPtr<UObject> FSGDTASerializerExtenderRegistry::GetSoftClassPtr(const FSGDTAClassId& ExtenderId) const
{
	const FSGDTASerializerExtenderRegistryEntry* effectiveEntry = GetEffectiveEntry(ExtenderId);
	if (effectiveEntry == nullptr)
	{
		return TSoftClassPtr<UObject>();
	}
	return effectiveEntry->Class;
}

TSoftClassPtr<UObject> FSGDTASerializerExtenderRegistry::GetSoftClassPtrByClassName(const FString& ClassName) const
{
	const FSGDTASerializerExtenderRegistryEntry* entry = FindByClassName(ClassName);
	if (entry == nullptr)
	{
		return TSoftClassPtr<UObject>();
	}
	return entry->Class;
}

int32 FSGDTASerializerExtenderRegistry::Num() const
{
	return Extenders.Num();
}

bool FSGDTASerializerExtenderRegistry::IsDirty() const
{
	return bIsDirty;
}

void FSGDTASerializerExtenderRegistry::Clear()
{
	Extenders.Empty();
	ExtenderIdIndex.Empty();
	ClassNameIndex.Empty();
	bIsDirty = false;
}

void FSGDTASerializerExtenderRegistry::ApplyServerOverrides(const TSharedPtr<FJsonObject>& ServerData)
{
	if (!ServerData.IsValid())
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
			TEXT("FSGDTASerializerExtenderRegistry::ApplyServerOverrides: Invalid server data"));
		return;
	}

	const TArray<TSharedPtr<FJsonValue>>* extendersArray = nullptr;
	if (!ServerData->TryGetArrayField(KEY_EXTENDERS, extendersArray) || extendersArray == nullptr)
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
			TEXT("FSGDTASerializerExtenderRegistry::ApplyServerOverrides: No '%s' array in server data"), *KEY_EXTENDERS);
		return;
	}

	for (const TSharedPtr<FJsonValue>& extenderValue : *extendersArray)
	{
		const TSharedPtr<FJsonObject>* extenderObjectPtr = nullptr;
		if (!extenderValue.IsValid() || !extenderValue->TryGetObject(extenderObjectPtr) || extenderObjectPtr == nullptr)
		{
			UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
				TEXT("FSGDTASerializerExtenderRegistry::ApplyServerOverrides: Skipping invalid server extender entry"));
			continue;
		}

		const TSharedPtr<FJsonObject>& extenderObject = *extenderObjectPtr;

		// ExtenderId is always required
		FString extenderIdString;
		if (!extenderObject->TryGetStringField(KEY_EXTENDER_ID, extenderIdString))
		{
			UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
				TEXT("FSGDTASerializerExtenderRegistry::ApplyServerOverrides: Skipping entry with missing extenderId"));
			continue;
		}

		FSGDTAClassId extenderId = FSGDTAClassId::FromString(extenderIdString);
		if (!extenderId.IsValid())
		{
			UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
				TEXT("FSGDTASerializerExtenderRegistry::ApplyServerOverrides: Skipping entry with invalid extenderId '%s'"),
				*extenderIdString);
			continue;
		}

		// ClassName - empty means "disable this extender"
		FString className;
		extenderObject->TryGetStringField(KEY_CLASS_NAME, className);

		if (className.IsEmpty())
		{
			// Empty className signals removal/disable - store a sentinel entry
			FSGDTASerializerExtenderRegistryEntry disabledEntry;
			disabledEntry.ExtenderId = extenderId;
			ServerOverlayEntries.Add(extenderId, MoveTemp(disabledEntry));
			continue;
		}

		// Build a soft class path from the class name
		FString classPath = FString::Printf(TEXT("/Script/%s"), *className);

		FSGDTASerializerExtenderRegistryEntry overlayEntry;
		overlayEntry.ExtenderId = extenderId;
		overlayEntry.ClassName = className;
		overlayEntry.Class = TSoftClassPtr<UObject>(FSoftObjectPath(classPath));

		ServerOverlayEntries.Add(extenderId, MoveTemp(overlayEntry));
	}

	UE_LOG(LogSGDynamicTextAssetsRuntime, Log,
		TEXT("FSGDTASerializerExtenderRegistry::ApplyServerOverrides: Applied %d server overlay entries"),
		ServerOverlayEntries.Num());
}

const FSGDTASerializerExtenderRegistryEntry* FSGDTASerializerExtenderRegistry::GetEffectiveEntry(
	const FSGDTAClassId& ExtenderId) const
{
	// Server overlay takes precedence
	const FSGDTASerializerExtenderRegistryEntry* overlayEntry = ServerOverlayEntries.Find(ExtenderId);
	if (overlayEntry != nullptr)
	{
		// Empty ClassName means the extender is disabled by the server
		if (overlayEntry->ClassName.IsEmpty())
		{
			return nullptr;
		}
		return overlayEntry;
	}

	// Fall through to local entry
	return FindByExtenderId(ExtenderId);
}

void FSGDTASerializerExtenderRegistry::ClearServerOverrides()
{
	ServerOverlayEntries.Empty();
}

bool FSGDTASerializerExtenderRegistry::HasServerOverrides() const
{
	return ServerOverlayEntries.Num() > 0;
}

void FSGDTASerializerExtenderRegistry::RebuildIndices()
{
	ExtenderIdIndex.Empty(Extenders.Num());
	ClassNameIndex.Empty(Extenders.Num());

	for (int32 i = 0; i < Extenders.Num(); ++i)
	{
		const FSGDTASerializerExtenderRegistryEntry& entry = Extenders[i];
		ExtenderIdIndex.Add(entry.ExtenderId, i);
		if (!entry.ClassName.IsEmpty())
		{
			ClassNameIndex.Add(entry.ClassName, i);
		}
	}
}

bool FSGDTASerializerExtenderRegistry::ParseExtenderEntry(const TSharedPtr<FJsonObject>& EntryObject,
	FSGDTASerializerExtenderRegistryEntry& OutEntry)
{
	FString extenderIdString;
	FString className;

	if (!EntryObject->TryGetStringField(KEY_EXTENDER_ID, extenderIdString)
		|| !EntryObject->TryGetStringField(KEY_CLASS_NAME, className))
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
			TEXT("FSGDTASerializerExtenderRegistry: Skipping extender entry with missing required fields"));
		return false;
	}

	FSGDTAClassId extenderId = FSGDTAClassId::FromString(extenderIdString);
	if (!extenderId.IsValid())
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
			TEXT("FSGDTASerializerExtenderRegistry: Skipping extender entry with invalid extenderId '%s'"),
			*extenderIdString);
		return false;
	}

	OutEntry.ExtenderId = extenderId;
	OutEntry.ClassName = className;

	// Use the full class path for reliable TSoftClassPtr resolution in packaged builds.
	FString classPath;
	if (EntryObject->TryGetStringField(KEY_CLASS_PATH, classPath) && !classPath.IsEmpty())
	{
		OutEntry.Class = TSoftClassPtr<UObject>(FSoftObjectPath(classPath));
	}
	else
	{
		UE_LOG(LogSGDynamicTextAssetsRuntime, Warning,
			TEXT("FSGDTASerializerExtenderRegistry: Entry '%s' has no classPath, class resolution may fail in packaged builds"),
			*className);
		OutEntry.Class = TSoftClassPtr<UObject>(FSoftObjectPath(className));
	}

	return true;
}
