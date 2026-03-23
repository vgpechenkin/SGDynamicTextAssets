# FSGSerializerFormat

[Back to Table of Contents](../TableOfContents.md)

## Overview

`FSGSerializerFormat` is a Blueprint-friendly USTRUCT wrapper around the raw `uint32` serializer type ID. It provides type safety, editor presentation, and Blueprint exposure for selecting and working with serializer formats throughout the plugin.

**Header:** `Source/SGDynamicTextAssetsRuntime/Public/Core/SGSerializerFormat.h`

## UPROPERTY Usage

### Single-Select Dropdown (Default)

Any `UPROPERTY` of type `FSGSerializerFormat` automatically renders as a dropdown picker in the Details panel:

```cpp
UPROPERTY(EditAnywhere, Category = "Serialization")
FSGSerializerFormat PreferredFormat;
```

The dropdown lists all registered serializer formats with their name and file extension. Includes a "None" option for clearing to invalid. A search bar appears when 10 or more formats are registered.

### Bitmask Mode

Add `meta=(SGDTABitmask)` to use a multi-select bitmask dropdown instead:

```cpp
UPROPERTY(EditAnywhere, Category = "Serialization", meta = (SGDTABitmask))
FSGSerializerFormat AllowedFormats;
```

The combo button shows a summary like "JSON | XML" or "None" or "All". Clicking opens a dropdown with checkmark-style toggle entries (matching the native Blueprint enum bitmask pattern). Includes "Select All" and "Clear All" entries at the top.

In bitmask mode, the TypeId stores a bitfield where each bit corresponds to a registered format's type ID (`1 << formatTypeId`). This limits bitmask mode to formats with TypeId < 32.

### Container Properties

`FSGSerializerFormat` works with all standard Unreal containers:

```cpp
// Array of formats - each element gets a dropdown picker
UPROPERTY(EditAnywhere, Category = "Serialization")
TArray<FSGSerializerFormat> FormatList;

// Set of formats - supported via GetTypeHash
UPROPERTY(EditAnywhere, Category = "Serialization")
TSet<FSGSerializerFormat> UniqueFormats;

// Map with format keys - each key gets a dropdown picker
UPROPERTY(EditAnywhere, Category = "Serialization")
TMap<FSGSerializerFormat, FString> FormatDescriptions;
```

## C++ API

### Construction

```cpp
// Default (invalid, TypeId=0)
FSGSerializerFormat format;

// From raw type ID
FSGSerializerFormat format = FSGSerializerFormat::FromTypeId(1);

// From file extension
FSGSerializerFormat format = FSGSerializerFormat::FromExtension(TEXT(".dta.json"));

// From serializer class constants
FSGSerializerFormat jsonFormat = FSGDynamicTextAssetJsonSerializer::FORMAT;
FSGSerializerFormat xmlFormat = FSGDynamicTextAssetXmlSerializer::FORMAT;
FSGSerializerFormat yamlFormat = FSGDynamicTextAssetYamlSerializer::FORMAT;

// Invalid constant
FSGSerializerFormat invalid = FSGSerializerFormat::INVALID;
```

### Querying

```cpp
// Validity
bool bValid = format.IsValid();  // true if TypeId != 0

// Raw type ID access
uint32 typeId = format.GetTypeId();

// Registry queries (resolve through registered serializers)
FText name = format.GetFormatName();           // "JSON", "XML", etc.
FString ext = format.GetFileExtension();        // ".dta.json", etc.
FText desc = format.GetFormatDescription();     // Full description
TSharedPtr<ISGDynamicTextAssetSerializer> serializer = format.GetSerializer();

// String representation
FString str = format.ToString();  // Returns TypeId as decimal string
```

### Comparison

```cpp
FSGSerializerFormat a = FSGDynamicTextAssetJsonSerializer::FORMAT;
FSGSerializerFormat b = FSGDynamicTextAssetXmlSerializer::FORMAT;

bool equal = (a == b);      // false
bool notEqual = (a != b);   // true
bool matchId = (a == 1u);   // true (compare with raw uint32)
```

### Serialization

`FSGSerializerFormat` supports all Unreal serialization paths:
- **FArchive** (binary): `Serialize(FArchive&)` reads/writes the uint32
- **Network**: `NetSerialize(FArchive&, UPackageMap*, bool&)` for replication
- **Text**: `ExportTextItem` / `ImportTextItem` for clipboard and config files
- **Hashing**: `GetTypeHash()` for use as TMap/TSet key

## Blueprint Functions

All functions are available in the "SG Dynamic Text Assets | Serializer Format" Blueprint category:

| Node | Description |
|------|-------------|
| Make Serializer Format | Construct from integer type ID |
| Make Serializer Format from Extension | Resolve from file extension string |
| Is Valid Serializer Format | Returns true if format is valid |
| Get Serializer Format Type Id | Returns the integer type ID |
| Get Serializer Format Name | Returns human-readable name |
| Get Serializer Format Extension | Returns file extension |
| Get Json Serializer Format | Returns the JSON format constant |
| Get Xml Serializer Format | Returns the XML format constant |
| Get Yaml Serializer Format | Returns the YAML format constant |
| == (Serializer Format) | Equality comparison |
| != (Serializer Format) | Inequality comparison |
| Get All Registered Serializer Formats | Returns array of all registered formats |

## Editor Customization Details

### Customization Class

`FSGSerializerFormatCustomization` (`IPropertyTypeCustomization`) handles both dropdown and bitmask modes. Registered for struct name `"SGSerializerFormat"` in the editor module.

### Dropdown Mode Behavior

- Queries `FSGDynamicTextAssetFileManager::GetAllRegisteredSerializers()` to populate entries
- Each row shows: format name + file extension in parentheses
- "None" entry at top for clearing
- Supports reset-to-default (listens via `SetOnPropertyValueChanged`)

### Bitmask Mode Behavior

- Detects `SGDTABitmask` meta tag on the property
- Uses `FMenuBuilder` with `EUserInterfaceActionType::Check` for native checkmark rendering
- Toggle: clicking a checked entry unchecks it (XOR on the bitfield)
- Menu stays open after toggling (allows multiple selections)
- "Select All" and "Clear All" entries appear above a separator

### Runtime Registration

Serializer formats are available for selection only after their serializers are registered with `FSGDynamicTextAssetFileManager`. The built-in JSON, XML, and YAML serializers register automatically during module startup. Third-party serializers appear in the dropdown after calling `FSGDynamicTextAssetFileManager::RegisterSerializer()`.

## Migration Guide: uint32 to FSGSerializerFormat

This section covers migrating from the deprecated `uint32` serializer type ID API to the new `FSGSerializerFormat` API.

### API Mapping Table

| Deprecated API | New API |
|---------------|---------|
| `ISGDynamicTextAssetSerializer::GetSerializerTypeId()` | `GetSerializerFormat()` |
| `ISGDynamicTextAssetSerializer::INVALID_SERIALIZER_TYPE_ID` | `FSGSerializerFormat::INVALID` |
| `FSGDynamicTextAssetJsonSerializer::TYPE_ID` | `FSGDynamicTextAssetJsonSerializer::FORMAT` |
| `FSGDynamicTextAssetXmlSerializer::TYPE_ID` | `FSGDynamicTextAssetXmlSerializer::FORMAT` |
| `FSGDynamicTextAssetYamlSerializer::TYPE_ID` | `FSGDynamicTextAssetYamlSerializer::FORMAT` |
| `FSGDynamicTextAssetFileManager::FindSerializerForTypeId(uint32)` | `FindSerializerForFormat(FSGSerializerFormat)` |
| `FSGDynamicTextAssetFileManager::GetTypeIdForExtension(FString)` | `GetFormatForExtension(FString)` |
| `USGDynamicTextAssetStatics::FindSerializerForTypeId(uint32)` | `FindSerializerForFormat(FSGSerializerFormat)` |
| `USGDynamicTextAssetStatics::GetTypeIdForExtension(FString)` | `GetFormatForExtension(FString)` |
| `FSGDynamicTextAssetBinarySerializer::BinaryReadSerializerTypeId(...)` | `BinaryReadSerializerFormat(...)` |
| `FSGBinaryEncodeParams::SerializerTypeId` | `SerializerFormat` |
| `FSGDynamicTextAssetFileInfo::SerializerTypeId` | `SerializerFormat` |
| `FSGDynamicTextAssetSerializerMetadata::SerializerTypeId` | `SerializerFormat` |

### Before/After Examples

**Creating a custom serializer class:**

```cpp
// BEFORE
class FMySerializer : public FSGDynamicTextAssetSerializerBase
{
public:
    static constexpr uint32 TYPE_ID = 100;
    virtual uint32 GetSerializerTypeId() const override { return TYPE_ID; }
};

// AFTER
class FMySerializer : public FSGDynamicTextAssetSerializerBase
{
public:
    static const FSGSerializerFormat FORMAT;
    virtual FSGSerializerFormat GetSerializerFormat() const override { return FORMAT; }
};
// In .cpp:
const FSGSerializerFormat FMySerializer::FORMAT = FSGSerializerFormat(100);
```

**Looking up a serializer:**

```cpp
// BEFORE
uint32 typeId = FSGDynamicTextAssetFileManager::GetTypeIdForExtension(TEXT(".dta.json"));
TSharedPtr<ISGDynamicTextAssetSerializer> serializer =
    FSGDynamicTextAssetFileManager::FindSerializerForTypeId(typeId);

// AFTER
FSGSerializerFormat format = FSGDynamicTextAssetFileManager::GetFormatForExtension(TEXT(".dta.json"));
TSharedPtr<ISGDynamicTextAssetSerializer> serializer =
    FSGDynamicTextAssetFileManager::FindSerializerForFormat(format);
```

**Checking for invalid format:**

```cpp
// BEFORE
if (serializerTypeId != ISGDynamicTextAssetSerializer::INVALID_SERIALIZER_TYPE_ID)
{
    // valid
}

// AFTER
if (serializerFormat.IsValid())
{
    // valid
}
```

**Reading binary file headers:**

```cpp
// BEFORE
uint32 serializerTypeId = 0;
FSGDynamicTextAssetBinarySerializer::BinaryReadSerializerTypeId(binaryData, serializerTypeId);
TSharedPtr<ISGDynamicTextAssetSerializer> serializer =
    FSGDynamicTextAssetFileManager::FindSerializerForTypeId(serializerTypeId);

// AFTER
FSGSerializerFormat serializerFormat;
FSGDynamicTextAssetBinarySerializer::BinaryReadSerializerFormat(binaryData, serializerFormat);
TSharedPtr<ISGDynamicTextAssetSerializer> serializer =
    FSGDynamicTextAssetFileManager::FindSerializerForFormat(serializerFormat);
```

### Binary Header Compatibility

`FSGBinaryDynamicTextAssetHeader` retains its `uint32 SerializerTypeId` field. The binary file format is a fixed 80-byte packed struct and cannot change without breaking existing cooked files. Conversion happens at the read/write boundary:

- **Writing:** `header.SerializerTypeId = params.SerializerFormat.GetTypeId()`
- **Reading:** `FSGSerializerFormat(header.SerializerTypeId)`

This is transparent to callers since `FSGBinaryEncodeParams` and `BinaryToString`/`BinaryReadSerializerFormat` use `FSGSerializerFormat` in their public APIs.

### Third-Party Serializer Plugin Migration

If you maintain a custom serializer plugin, update your serializer class:

1. Replace `static constexpr uint32 TYPE_ID = N` with `static const FSGSerializerFormat FORMAT` (defined in .cpp as `FSGSerializerFormat(N)`)
2. Replace `virtual uint32 GetSerializerTypeId() const override` with `virtual FSGSerializerFormat GetSerializerFormat() const override`
3. Update any code that calls `FindSerializerForTypeId` to use `FindSerializerForFormat`
4. Update any `FSGBinaryEncodeParams::SerializerTypeId` assignments to use `SerializerFormat`
5. Add `#include "Core/SGSerializerFormat.h"` to your serializer header

### Deprecation Timeline

All deprecated APIs are marked with `UE_DEPRECATED(5.6, ...)` and will be removed in UE 5.7. The deprecated wrappers call through to the new implementations, so both APIs produce identical behavior during the transition period.
