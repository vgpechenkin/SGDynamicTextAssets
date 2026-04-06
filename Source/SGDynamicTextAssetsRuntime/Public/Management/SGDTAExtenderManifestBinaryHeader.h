// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Ensuring binary layout across platforms
// as a safety measure by making alignment boundary 1
#pragma pack(push, 1)
/**
 * Header structure for the binary extender manifest file (DTA_{Key}_extenders.dta.bin).
 * Fixed 32-byte header for quick validation and file info access.
 *
 * The binary extender manifest maps extender class IDs to class names and
 * class paths using a compact binary format with a string table
 * for deduplication.
 */
struct SGDYNAMICTEXTASSETSRUNTIME_API FSGDTAExtenderManifestBinaryHeader
{
public:

	FSGDTAExtenderManifestBinaryHeader();

	/** Returns true if the magic number matches the expected "DTEM" value. */
	bool IsValid() const;

	/** Returns true if the ContentHash field is non-zero (file has integrity check). */
	bool HasContentHash() const;

	/** Returns the magic number as a hex string (from instance data). */
	FString GetMagicNumberString() const;

	/** Returns the expected magic number as a hex string (static). */
	static FString GetExpectedMagicNumberString();

	/** Serializes the header to/from an archive. */
	void Serialize(FArchive& Ar);

	/**
	 * 4 bytes for "DTEM" (short for Dynamic Text Extender Manifest).
	 * (0x44 = 'D', 0x54 = 'T', 0x45 = 'E', 0x4D = 'M')
	 *
	 * Increment CURRENT_VERSION if this changes.
	 */
	static constexpr uint8 MAGIC_NUMBER[] = { 0x44, 0x54, 0x45, 0x4D };

	/** Current binary manifest format version. */
	static constexpr uint32 CURRENT_VERSION = 1;

	/** Size of the truncated SHA-1 content hash in bytes (96 bits). */
	static constexpr uint32 CONTENT_HASH_SIZE = 12;

	/** Total header size in bytes. */
	static constexpr uint32 HEADER_SIZE = 32;

	/**
	 * 4 bytes
	 *
	 * Magic hexadecimal number identifying this as a DTA extender manifest: "DTEM"
	 */
	uint8 MagicNumber[sizeof(MAGIC_NUMBER)];

	/**
	 * 4 bytes
	 *
	 * Binary manifest format version for compatibility checks.
	 */
	uint32 Version = CURRENT_VERSION;

	/**
	 * 4 bytes
	 *
	 * Number of extender entries following the header.
	 */
	uint32 EntryCount = 0;

	/**
	 * 4 bytes
	 *
	 * Byte offset from the start of the file to the string table.
	 * Used by the loader to seek directly to the string table for deserialization.
	 */
	uint32 StringTableOffset = 0;

	/**
	 * 4 bytes
	 *
	 * Reserved for future use. Must be zero-filled.
	 */
	uint8 Reserved[4] = { 0 };

	/**
	 * 12 bytes
	 *
	 * Truncated SHA-1 hash of the entries and string table raw bytes
	 * for integrity verification. Computed over all bytes after the header
	 * up to end of string table. If all zeros, hash validation is skipped.
	 */
	uint8 ContentHash[CONTENT_HASH_SIZE] = { 0 };
};
// Return the pack to normal behavior
#pragma pack(pop)
