// Copyright Start Games, Inc. All Rights Reserved.

#include "Core/SGDynamicTextAsset.h"

#include "SGDynamicTextAssetLogs.h"
#include "AssetRegistry/AssetData.h"
#include "AssetRegistry/AssetRegistryModule.h"

USGDynamicTextAsset::USGDynamicTextAsset()
{
    // DynamicTextAssetId will be set by the subsystem when loading
    // UserFacingId will be populated from JSON
    // Version will be populated from JSON
}

const FSGDynamicTextAssetId& USGDynamicTextAsset::GetDynamicTextAssetId() const
{
    return DynamicTextAssetId;
}

void USGDynamicTextAsset::BP_PostDynamicTextAssetLoaded_Implementation()
{
    // Default implementation does nothing
    // Subclasses can override for custom initialization
}

void USGDynamicTextAsset::ValidateSoftPathsInProperty(const FProperty* Property,
    const void* ContainerPtr,
    const FString& PropertyPath,
    FSGDynamicTextAssetValidationResult& OutResult) const
{
    if (!Property)
    {
        UE_LOG(LogSGDynamicTextAssetsRuntime, Error, TEXT("ValidateSoftPathsInProperty: Inputted NULL Property"));
        return;
    }
    if (!ContainerPtr)
    {
        UE_LOG(LogSGDynamicTextAssetsRuntime, Error, TEXT("ValidateSoftPathsInProperty: Inputted NULL ContainerPtr"));
        return;
    }

    auto validateAssetPath = [&OutResult, &PropertyPath](const FSoftObjectPath& AssetPath)
    {
        if (AssetPath.IsValid() && !AssetPath.IsNull())
        {
            FAssetRegistryModule& assetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
            FAssetData assetData = assetRegistryModule.Get().GetAssetByObjectPath(AssetPath);
            
            // If it failed to find the asset, check if it's a Blueprint Class reference ending in "_C"
            if (!assetData.IsValid())
            {
                FString pathString = AssetPath.ToString();
                if (pathString.EndsWith(TEXT("_C")))
                {
                    pathString.RemoveFromEnd(TEXT("_C"));
                    FSoftObjectPath strippedPath(pathString);
                    assetData = assetRegistryModule.Get().GetAssetByObjectPath(strippedPath);
                }
            }

            // If still not found, check if this is a sub-object path (e.g., a level actor instance).
            // Sub-object paths use ':' to separate the top-level asset from the sub-object within it.
            // The Asset Registry only tracks top-level assets, so validate the parent asset instead.
            // Examples:
            //   /Game/Lvl_Basic.Lvl_Basic:PersistentLevel.BP_TestActor_C_2 -> validates /Game/Lvl_Basic.Lvl_Basic
            //   /Game/SomeBP.SomeBP:MyComponent -> validates /Game/SomeBP.SomeBP
            if (!assetData.IsValid())
            {
                FString subPathString = AssetPath.GetSubPathString();
                if (!subPathString.IsEmpty())
                {
                    FSoftObjectPath parentPath(AssetPath.GetAssetPath(), FString());
                    assetData = assetRegistryModule.Get().GetAssetByObjectPath(parentPath);
                }
            }

            if (!assetData.IsValid())
            {
                OutResult.AddError(
                    FText::Format(FText::FromString(TEXT("Missing asset for path: {0}")), FText::FromString(AssetPath.ToString())),
                    PropertyPath,
                    FText::FromString(TEXT("The asset referenced by this soft path could not be found in the Asset Registry. It may have been deleted, moved, or renamed."))
                );
            }
        }
    };

    // Instanced object properties own sub-objects whose properties may contain soft paths.
    // Walk into the sub-object and recurse on its properties.
    if (const FObjectProperty* objectProp = CastField<FObjectProperty>(Property))
    {
        if (Property->HasAllPropertyFlags(CPF_InstancedReference))
        {
            const void* valuePtr = objectProp->ContainerPtrToValuePtr<void>(ContainerPtr);
            if (const UObject* subObject = objectProp->GetObjectPropertyValue(valuePtr))
            {
                for (TFieldIterator<FProperty> innerIt(subObject->GetClass()); innerIt; ++innerIt)
                {
                    const FProperty* innerProp = *innerIt;
                    FString nestedPath = FString::Printf(TEXT("%s.%s"), *PropertyPath, *innerProp->GetName());
                    ValidateSoftPathsInProperty(innerProp, subObject, nestedPath, OutResult);
                }
            }
            return;
        }
    }

    if (const FSoftObjectProperty* softObjProp = CastField<FSoftObjectProperty>(Property))
    {
        const void* valuePtr = softObjProp->ContainerPtrToValuePtr<void>(ContainerPtr);
        if (const FSoftObjectPtr* softPtr = static_cast<const FSoftObjectPtr*>(valuePtr))
        {
            validateAssetPath(softPtr->ToSoftObjectPath());
        }
        return;
    }

    if (const FSoftClassProperty* softClassProp = CastField<FSoftClassProperty>(Property))
    {
        const void* valuePtr = softClassProp->ContainerPtrToValuePtr<void>(ContainerPtr);
        if (const FSoftObjectPtr* softPtr = static_cast<const FSoftObjectPtr*>(valuePtr))
        {
            validateAssetPath(softPtr->ToSoftObjectPath());
        }
        return;
    }

    if (const FStructProperty* structProp = CastField<FStructProperty>(Property))
    {
        const void* structPtr = structProp->ContainerPtrToValuePtr<void>(ContainerPtr);
        for (TFieldIterator<FProperty> innerIt(structProp->Struct); innerIt; ++innerIt)
        {
            const FProperty* innerProp = *innerIt;
            FString nestedPath = FString::Printf(TEXT("%s.%s"), *PropertyPath, *innerProp->GetName());
            ValidateSoftPathsInProperty(innerProp, structPtr, nestedPath, OutResult);
        }
        return;
    }

    if (const FArrayProperty* arrayProp = CastField<FArrayProperty>(Property))
    {
        const void* arrayPtr = arrayProp->ContainerPtrToValuePtr<void>(ContainerPtr);
        FScriptArrayHelper arrayHelper(arrayProp, arrayPtr);

        for (int32 index = 0; index < arrayHelper.Num(); ++index)
        {
            const void* elementPtr = arrayHelper.GetRawPtr(index);
            FString elementPath = FString::Printf(TEXT("%s[%d]"), *PropertyPath, index);
            ValidateSoftPathsInProperty(arrayProp->Inner, elementPtr, elementPath, OutResult);
        }
        return;
    }

    if (const FMapProperty* mapProp = CastField<FMapProperty>(Property))
    {
        const void* mapPtr = mapProp->ContainerPtrToValuePtr<void>(ContainerPtr);
        FScriptMapHelper mapHelper(mapProp, mapPtr);

        for (FScriptMapHelper::FIterator it = mapHelper.CreateIterator(); it; ++it)
        {
            const void* valuePtr = mapHelper.GetValuePtr(it.GetInternalIndex());
            FString valuePath = FString::Printf(TEXT("%s[Value]"), *PropertyPath);
            ValidateSoftPathsInProperty(mapProp->ValueProp, valuePtr, valuePath, OutResult);
        }
    }
}

void USGDynamicTextAsset::SetDynamicTextAssetId(const FSGDynamicTextAssetId& InId)
{
    DynamicTextAssetId = InId;
#if WITH_EDITOR
    OnDynamicTextAssetIdChanged.Broadcast(this);
#endif
}

const FString& USGDynamicTextAsset::GetUserFacingId() const
{
    return UserFacingId;
}

void USGDynamicTextAsset::SetUserFacingId(const FString& InUserFacingId)
{
    UserFacingId = InUserFacingId;
#if WITH_EDITOR
    OnUserFacingIdChanged.Broadcast(this);
#endif
}

const FSGDynamicTextAssetVersion& USGDynamicTextAsset::GetVersion() const
{
    return Version;
}

int32 USGDynamicTextAsset::GetCurrentMajorVersion() const
{
    // Using default value
    return 1;
}

void USGDynamicTextAsset::SetVersion(const FSGDynamicTextAssetVersion& InVersion)
{
    Version = InVersion;
#if WITH_EDITOR
    OnVersionChanged.Broadcast(this);
#endif
}

void USGDynamicTextAsset::PostDynamicTextAssetLoaded()
{
    BP_PostDynamicTextAssetLoaded();
}

bool USGDynamicTextAsset::Native_ValidateDynamicTextAsset(FSGDynamicTextAssetValidationResult& OutResult) const
{
    if (!DynamicTextAssetId.IsValid())
    {
        OutResult.AddError(
            FText::FromString(TEXT("Dynamic text asset has invalid ID")),
            TEXT("DynamicTextAssetId"),
            FText::FromString(TEXT("Recreate this dynamic text asset to generate a valid ID"))
        );
    }

    if (UserFacingId.IsEmpty())
    {
        OutResult.AddError(
            FText::FromString(TEXT("Dynamic text asset has empty UserFacingId")),
            TEXT("UserFacingId"),
            FText::FromString(TEXT("Set a unique user-facing name for this dynamic text asset"))
        );
    }

    if (!Version.IsValid())
    {
        OutResult.AddError(
            FText::FromString(TEXT("Dynamic text asset has invalid version numbers")),
            TEXT("Version"),
            FText::FromString(TEXT("Version numbers must be non-negative"))
        );
    }

    // Validate soft asset references internally
    for (TFieldIterator<FProperty> propertyItr(GetClass()); propertyItr; ++propertyItr)
    {
        const FProperty* property = *propertyItr;
        ValidateSoftPathsInProperty(property, this, property->GetName(), OutResult);
    }

    ValidateDynamicTextAsset(OutResult);

    return OutResult.IsValid();
}

bool USGDynamicTextAsset::MigrateFromVersion(
    const FSGDynamicTextAssetVersion& OldVersion,
    const FSGDynamicTextAssetVersion& CurrentVersion,
    const TSharedPtr<FJsonObject>& OldData)
{
    // Default implementation: no migration needed
    // Subclasses should override to handle breaking changes between versions
    return true;
}

FString USGDynamicTextAsset::GetVersionString() const
{
    return Version.ToString();
}

TSet<FName> USGDynamicTextAsset::GetMetadataPropertyNames()
{
    // GET_MEMBER_NAME_CHECKED is called here (inside a member function body) so that it has
    // access to the private members DynamicTextAssetId, UserFacingId, and Version.
    // If any of these properties are renamed, this function will produce a compile error.
    return {
        GET_MEMBER_NAME_CHECKED(USGDynamicTextAsset, DynamicTextAssetId),
        GET_MEMBER_NAME_CHECKED(USGDynamicTextAsset, UserFacingId),
        GET_MEMBER_NAME_CHECKED(USGDynamicTextAsset, Version)
    };
}