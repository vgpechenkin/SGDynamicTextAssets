// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonValue.h"
#include "Serialization/SGDynamicTextAssetSerializer.h"

class FObjectProperty;

/**
 * Abstract base class for serializers that use FJsonValue as an intermediate
 * representation for UE property conversion (the "JSON intermediate" approach).
 *
 * Provides three protected virtual helpers used by SerializeProvider and
 * DeserializeProvider implementations:
 * - SerializePropertyToValue: FProperty -> FJsonValue (default: FJsonObjectConverter)
 * - DeserializeValueToProperty: FJsonValue -> FProperty (default: FJsonObjectConverter)
 * - ShouldSerializeProperty: Filtering predicate (default: skip metadata + deprecated)
 *
 * Also provides instanced object helpers for UPROPERTY(Instanced) sub-objects:
 * - SerializeInstancedObjectToValue: UObject sub-object -> FJsonValue (recursive)
 * - DeserializeValueToInstancedObject: FJsonValue -> UObject sub-object (recursive)
 * - IsInstancedObjectProperty: Static helper to detect CPF_InstancedReference
 *
 * All ISGDynamicTextAssetSerializer pure virtuals remain unimplemented here.
 * Concrete serializers (FSGDynamicTextAssetJsonSerializer, FSGDynamicTextAssetXmlSerializer, etc.)
 * inherit from this class and implement the format-specific methods.
 *
 * To bypass the JSON intermediate and convert properties directly to the target format,
 * override SerializePropertyToValue and DeserializeValueToProperty.
 */
class SGDYNAMICTEXTASSETSRUNTIME_API FSGDynamicTextAssetSerializerBase : public ISGDynamicTextAssetSerializer
{
public:

    virtual ~FSGDynamicTextAssetSerializerBase() = default;

    /**
     * Reserved JSON key used to store the UClass name of an instanced sub-object.
     * Single source of truth for all serialization formats (JSON, XML, YAML).
     */
    static const FString INSTANCED_OBJECT_CLASS_KEY;

    /**
     * Checks whether a property has the CPF_InstancedReference flag,
     * indicating it is a UPROPERTY(Instanced) sub-object owned inline.
     *
     * @param InProperty The property to check
     * @return True if the property has CPF_InstancedReference set
     */
    static bool IsInstancedObjectProperty(const FProperty* InProperty);

    /**
     * Resolves a UClass from a full class path string.
     *
     * Uses LoadObject to resolve paths produced by UClass::GetPathName(),
     * supporting both C++ classes ("/Script/Module.ClassName") and Blueprint
     * classes ("/Game/Path/BP_Name.BP_Name_C"). Triggers asset loading for
     * Blueprint classes not yet in memory.
     *
     * @param ClassIdentifier Full class path from GetPathName()
     * @return The resolved UClass, or nullptr if not found
     */
    static UClass* ResolveInstancedObjectClass(const FString& ClassIdentifier);

    /**
     * Recursively collects all UClass references used by instanced sub-objects
     * on the given UObject. Scans single, TArray, TSet, and TMap instanced
     * properties and recurses into nested instanced sub-objects.
     *
     * @param Object The UObject to scan for instanced sub-objects
     * @param OutClasses Set to populate with discovered UClass pointers
     */
    static void CollectInstancedObjectClasses(const UObject* Object, TSet<TObjectPtr<UClass>>& OutClasses);

protected:

    /**
     * Converts a single UPROPERTY value to its FJsonValue intermediate form.
     *
     * Default implementation calls FJsonObjectConverter::UPropertyToJsonValue.
     * Override to provide a custom conversion strategy (e.g., a direct XML or YAML node).
     *
     * @param Property The property descriptor (non-const - required by FJsonObjectConverter)
     * @param ValuePtr Const pointer to the property's value inside the containing object
     * @return The FJsonValue intermediate, or nullptr if conversion fails
     */
    virtual TSharedPtr<FJsonValue> SerializePropertyToValue(FProperty* Property, const void* ValuePtr) const;

    /**
     * Converts a FJsonValue intermediate form back into a UPROPERTY value.
     *
     * Default implementation calls FJsonObjectConverter::JsonValueToUProperty.
     * Override to provide a custom conversion strategy.
     *
     * @param Value The FJsonValue to convert (must be valid - callers must check IsValid() first)
     * @param Property The property descriptor
     * @param ValuePtr Non-const pointer to the property's value storage in the containing object
     * @return True if conversion succeeded
     */
    virtual bool DeserializeValueToProperty(const TSharedPtr<FJsonValue>& Value, FProperty* Property, void* ValuePtr) const;

    /**
     * Returns true if the property should be included in serialization and deserialization.
     *
     * Default implementation returns false (excludes) for:
     * - USGDynamicTextAsset base metadata properties: DynamicTextAssetId, Version, UserFacingId.
     * These are stored as wrapper-level fields, not inside the data block.
     * - Properties flagged as CPF_Deprecated.
     * Deprecated properties are excluded from BOTH serialization and deserialization.
     * If an existing file contains a value for a now-deprecated property, that value
     * is silently ignored when loading. This is intentional.
     *
     * Override to add format-specific or class-specific filtering rules.
     *
     * @param Property The property to evaluate
     * @return True if the property should be serialized/deserialized
     */
    virtual bool ShouldSerializeProperty(const FProperty* Property) const;

    /**
     * Serializes a UPROPERTY(Instanced) sub-object to its FJsonValue intermediate form.
     *
     * If SubObject is null, returns FJsonValueNull. Otherwise creates a FJsonObject
     * with INSTANCED_OBJECT_CLASS_KEY set to the sub-object's class name, then
     * iterates its UProperties (filtered by ShouldSerializeProperty) and recursively
     * serializes each via SerializePropertyToValue. Nested instanced objects are
     * handled by recursion.
     *
     * @param Property The FObjectProperty descriptor for the instanced property
     * @param SubObject The instanced sub-object to serialize (can be nullptr)
     * @return FJsonValue representing the sub-object, or FJsonValueNull for nullptr
     */
    virtual TSharedPtr<FJsonValue> SerializeInstancedObjectToValue(
        const FObjectProperty* Property,
        const UObject* SubObject) const;

    /**
     * Deserializes a FJsonValue back into a UPROPERTY(Instanced) sub-object.
     *
     * If Value is null type, sets the property to nullptr and returns true.
     * Otherwise reads the JSON object, extracts INSTANCED_OBJECT_CLASS_KEY to resolve
     * the UClass, creates a new sub-object via NewObject, and recursively deserializes
     * its properties via DeserializeValueToProperty. Nested instanced objects are
     * handled by recursion.
     *
     * @param Value The FJsonValue to deserialize (null type is valid for nullptr sub-objects)
     * @param Property The FObjectProperty descriptor for the instanced property
     * @param PropertyValuePtr Pointer to the property's storage (the UObject* slot)
     * @param Outer The outer UObject that will own the created sub-object
     * @return True if deserialization succeeded
     */
    virtual bool DeserializeValueToInstancedObject(
        const TSharedPtr<FJsonValue>& Value,
        const FObjectProperty* Property,
        void* PropertyValuePtr,
        UObject* Outer) const;
};
