// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/SGSerializerFormat.h"
#include "UObject/Object.h"

#include "SGSerializerFormatCustomizationTestTypes.generated.h"

/**
 * Test UObject with FSGSerializerFormat properties for verifying
 * UPROPERTY reflection and meta tag compilation.
 */
UCLASS(NotBlueprintable, NotBlueprintType, MinimalAPI, Hidden)
class USGSerializerFormatCustomizationTestObject : public UObject
{
	GENERATED_BODY()

public:

	/** Standard single-select format property. */
	UPROPERTY(EditAnywhere, Category = "Test")
	FSGSerializerFormat SingleFormat;

	/** Bitmask format property for multi-select. */
	UPROPERTY(EditAnywhere, Category = "Test", meta = (SGDTABitmask))
	FSGSerializerFormat BitmaskFormat;
};
