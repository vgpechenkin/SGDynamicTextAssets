// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "UObject/Class.h"
#include "UObject/SoftObjectPtr.h"
#include "Misc/Guid.h"

#include "SGDTAClassId.generated.h"

/**
 * General-purpose unique identity wrapper for UObject classes.
 *
 * Wraps a raw FGuid in a dedicated type so that class identity is
 * distinguishable at the type level from both arbitrary GUIDs and from
 * FSGDynamicTextAssetId / FSGDynamicTextAssetTypeId. This separation
 * prevents accidental misuse between different GUID domains.
 *
 * Used by the serializer extender registry and other manifest systems
 * to map a stable GUID to any UObject-derived class, enabling identity
 * to be decoupled from C++ class names.
 *
 * Supports binary serialization, network serialization, and text
 * export/import for clipboard and config round-tripping.
 *
 * --------------------------------------------------
 * META TAGS
 *
 * - EDITOR ONLY -
 * SGDTAClassType: Filter the class type that this class ID can select in the editor.
 * Works only with UObject based types without the U prefix.
 *     Example: UPROPERTY(meta = (SGDTAClassType = "MyCustomType"))
 */
USTRUCT(BlueprintType)
struct SGDYNAMICTEXTASSETSRUNTIME_API FSGDTAClassId
{
	GENERATED_BODY()
public:

	FSGDTAClassId() = default;

	FSGDTAClassId(const FGuid& InGuid);

	explicit FSGDTAClassId(FGuid&& InGuid);

	/** Returns true if the underlying GUID is valid (non-zero). */
	bool IsValid() const;

	/** Returns the underlying FGuid. */
	const FGuid& GetGuid() const;

	/** Sets the underlying GUID. */
	void SetGuid(const FGuid& InGuid);

	/**
	 * Converts a string to a GUID and stores it (if one GUID is already set then it will be overridden).
	 * @return true if the string was converted successfully, false otherwise.
	 */
	bool ParseString(const FString& GuidString);

	/** Resets to an invalid (all-zero) state. */
	void Invalidate();

	/**
	 * Returns the GUID as a hyphenated string.
	 * Example: "550E8400-E29B-41D4-A716-446655440000"
	 */
	FString ToString() const;

	/** Generates a brand-new unique class ID in place. */
	void GenerateNewId();

	/** Generates a brand-new unique class ID. */
	static FSGDTAClassId NewGeneratedId();

	/** Constructs from an existing FGuid. */
	static FSGDTAClassId FromGuid(const FGuid& InGuid);

	/**
	 * Parses a class ID from a string.
	 * Returns an invalid ID if parsing fails.
	 *
	 * @param InString The string to parse (hyphenated GUID format)
	 * @return The parsed FSGDTAClassId, or an invalid ID on failure
	 */
	static FSGDTAClassId FromString(const FString& InString);

	/**
	 * Resolves the UObject class associated with this class ID
	 * using the provided registry.
	 *
	 * RegistryType must provide:
	 *   TSoftClassPtr<UObject> GetSoftClassPtr(const FSGDTAClassId&) const
	 *
	 * @tparam RegistryType The registry type to resolve through
	 * @param Registry The registry instance to query
	 * @return Soft class pointer to the resolved class, or empty if not found
	 */
	template<typename RegistryType>
	TSoftClassPtr<UObject> GetClass(const RegistryType& Registry) const
	{
		if (!IsValid())
		{
			return TSoftClassPtr<UObject>();
		}
		return Registry.GetSoftClassPtr(*this);
	}

	/**
	 * Typed version of GetClass that resolves and casts the result.
	 *
	 * @tparam ClassType The expected UObject-derived base class type
	 * @tparam RegistryType The registry type to resolve through
	 * @param Registry The registry instance to query
	 * @return Soft class pointer cast to the requested type
	 */
	template<typename ClassType, typename RegistryType>
	TSoftClassPtr<ClassType> GetClassTyped(const RegistryType& Registry) const
	{
		static_assert(TIsDerivedFrom<ClassType, UObject>::Value, "ClassType must derive from UObject.");
		TSoftClassPtr<UObject> resolved = GetClass<RegistryType>(Registry);
		if (resolved.IsNull())
		{
			return TSoftClassPtr<ClassType>();
		}
		return TSoftClassPtr<ClassType>(resolved.ToSoftObjectPath());
	}

	void operator=(const FGuid& Other);

	bool operator==(const FGuid& Other) const;
	bool operator!=(const FGuid& Other) const;
	bool operator==(const FSGDTAClassId& Other) const;
	bool operator!=(const FSGDTAClassId& Other) const;

	friend uint32 GetTypeHash(const FSGDTAClassId& Id)
	{
		return GetTypeHash(Id.Guid);
	}

	/**
	 * Binary archive serialization.
	 * Serializes the underlying GUID directly without property tags.
	 *
	 * @param Ar The archive to serialize to/from
	 * @return True (serialization always succeeds for this simple struct)
	 */
	bool Serialize(FArchive& Ar);

	/**
	 * Network replication serialization.
	 *
	 * @param Ar The archive to serialize to/from
	 * @param Map Package map for name/object resolution (unused)
	 * @param bOutSuccess Set to true on success
	 * @return True if serialization succeeded
	 */
	bool NetSerialize(FArchive& Ar, UPackageMap* Map, bool& bOutSuccess);

	/**
	 * Exports the ID as a text string for clipboard and config usage.
	 *
	 * @param ValueStr Output string to append the exported value to
	 * @param DefaultValue Default value for comparison (unused)
	 * @param Parent Owning UObject (unused)
	 * @param PortFlags Export flags
	 * @param ExportRootScope Root export scope (unused)
	 * @return True if export succeeded
	 */
	bool ExportTextItem(FString& ValueStr, const FSGDTAClassId& DefaultValue, UObject* Parent, int32 PortFlags, UObject* ExportRootScope) const;

	/**
	 * Imports the ID from a text string.
	 *
	 * @param Buffer Input text buffer (advanced past consumed characters)
	 * @param PortFlags Import flags
	 * @param Parent Owning UObject (unused)
	 * @param ErrorText Output for error messages
	 * @return True if import succeeded
	 */
	bool ImportTextItem(const TCHAR*& Buffer, int32 PortFlags, UObject* Parent, FOutputDevice* ErrorText);

#if WITH_EDITOR

	DECLARE_MULTICAST_DELEGATE_TwoParams(FSGDTAClassIdGuidChangeSignature, FGuid/*Previous*/, FGuid/*New*/);
	/**
	 * For editor tooling, broadcasts when this class ID's GUID is changed.
	 * Param 1 = Previous GUID
	 * Param 2 = New/Current GUID
	 */
	FSGDTAClassIdGuidChangeSignature OnGuidChanged_Editor;

#endif

	/** Invalid Class ID. */
	static const FSGDTAClassId INVALID_CLASS_ID;

private:

	/** The underlying unique identifier. */
	UPROPERTY(Transient)
	FGuid Guid;
};

template<>
struct TStructOpsTypeTraits<FSGDTAClassId> : public TStructOpsTypeTraitsBase2<FSGDTAClassId>
{
	enum
	{
		WithSerializer = true,
		WithNetSerializer = true,
		WithExportTextItem = true,
		WithImportTextItem = true,
		WithIdenticalViaEquality = true
	};
};
