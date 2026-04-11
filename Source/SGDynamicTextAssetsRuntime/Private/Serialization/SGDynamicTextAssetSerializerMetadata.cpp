// Copyright Start Games, Inc. All Rights Reserved.

#include "Serialization/SGDynamicTextAssetSerializerMetadata.h"

bool FSGDynamicTextAssetSerializerMetadata::IsValidId() const
{
	return SerializerFormat.IsValid();
}
