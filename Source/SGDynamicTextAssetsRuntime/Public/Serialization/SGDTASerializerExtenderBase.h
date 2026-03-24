// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "SGDTASerializerExtenderBase.generated.h"

/**
 * Abstract base class for all serializer extenders.
 *
 * Serializer extenders provide pluggable behavior for specific aspects of
 * the DTA serialization pipeline. Concrete extender types (such as
 * USGAssetBundleExtender) define their own virtual interface for the
 * behavior they extend.
 *
 * Identity is managed externally by the serializer extender registry
 * (FSGDTAClassId mapping). The base class itself carries no data members
 * and exists purely as a common type root for the extender hierarchy.
 *
 * Whether callers use the CDO or create instances is a design choice
 * per extender type and is not enforced by this base class.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, ClassGroup = "Start Games")
class SGDYNAMICTEXTASSETSRUNTIME_API USGDTASerializerExtenderBase : public UObject
{
	GENERATED_BODY()
public:

	USGDTASerializerExtenderBase() = default;
};
