// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Serialization/SGDTAAssetBundleExtender.h"

#include "SGDTAAssetBundleExtenderTests.generated.h"

UCLASS(NotBlueprintable, NotBlueprintType, MinimalAPI, Hidden)
class USGDTATestAssetBundleExtenderA : public USGDTAAssetBundleExtender
{
	GENERATED_BODY()
};

UCLASS(NotBlueprintable, NotBlueprintType, MinimalAPI, Hidden)
class USGDTATestAssetBundleExtenderB : public USGDTAAssetBundleExtender
{
	GENERATED_BODY()
};
