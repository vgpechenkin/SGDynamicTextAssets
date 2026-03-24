// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/SGDTASerializerFormat.h"
#include "UObject/Object.h"

#include "SGDTASerializerFormatCustomizationTestTypes.generated.h"

/**
 * Test UObject with FSGDTASerializerFormat properties for verifying
 * UPROPERTY reflection and meta tag compilation.
 */
UCLASS(NotBlueprintable, NotBlueprintType, MinimalAPI, Hidden)
class USGDTASerializerFormatCustomizationTestObject : public UObject
{
	GENERATED_BODY()

public:

	/** Standard single-select format property. */
	UPROPERTY(EditAnywhere, Category = "Test")
	FSGDTASerializerFormat SingleFormat;

	/** Bitmask format property for multi-select. */
	UPROPERTY(EditAnywhere, Category = "Test", meta = (SGDTABitmask))
	FSGDTASerializerFormat BitmaskFormat;
};
