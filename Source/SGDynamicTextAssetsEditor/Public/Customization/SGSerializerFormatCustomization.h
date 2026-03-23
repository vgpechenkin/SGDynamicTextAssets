// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/SGSerializerFormat.h"
#include "IPropertyTypeCustomization.h"
#include "Widgets/Views/SListView.h"

class SComboButton;
class SImage;
class SSearchBox;
class STextBlock;
class SWidget;
struct FSlateBrush;

/**
 * Unified property type customization for FSGSerializerFormat.
 *
 * Operates in two modes based on the presence of the SGDTABitmask meta tag:
 *
 * **Dropdown mode** (default, no meta tag):
 * Shows a combo button with a dropdown list of registered serializer formats.
 * Each entry displays icon, format name, and file extension.
 * Includes a "None" option and search bar when 10+ formats are registered.
 *
 * **Bitmask mode** (meta=(SGDTABitmask)):
 * Shows a checkbox panel where multiple formats can be selected.
 * The uint32 TypeId is treated as a bitfield where each bit corresponds
 * to a registered format's type ID (bit = 1 << typeId).
 * Header shows summary text like "JSON, XML" or "None" or "All".
 */
class FSGSerializerFormatCustomization : public IPropertyTypeCustomization
{
public:

	/** Entry in the format picker dropdown (dropdown mode). */
	struct FFormatEntry
	{
		FSGSerializerFormat Format;
		FText Name;
		FString Extension;
		const FSlateBrush* IconBrush = nullptr;
		uint8 bIsChecked : 1 = 0;
	};

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();

	// IPropertyTypeCustomization overrides
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
		FDetailWidgetRow& HeaderRow,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
		IDetailChildrenBuilder& ChildBuilder,
		IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	// ~IPropertyTypeCustomization overrides

private:

	// -- Shared --

	/** Populates FormatEntries from the serializer registry. */
	void PopulateFormatEntries();

	/** Called when the property value changes externally (e.g. reset-to-default). */
	void OnPropertyValueChanged();

	// -- Dropdown mode --

	/** Reads the current FSGSerializerFormat value from the property handle. */
	void ReadCurrentValue();

	/** Writes an updated FSGSerializerFormat back to the property handle. */
	void WriteValue(const FSGSerializerFormat& NewFormat);

	/** Returns the display text for the currently selected format. */
	FText GetCurrentDisplayText() const;

	/** Returns the tooltip for the currently selected format. */
	FText GetCurrentTooltipText() const;

	/** Returns the icon brush for the currently selected format. */
	const FSlateBrush* GetCurrentIconBrush() const;

	/** Generates the dropdown menu content. */
	TSharedRef<SWidget> GeneratePickerMenu();

	/** Generates a single row in the format list. */
	TSharedRef<ITableRow> GenerateListRow(
		TSharedPtr<FFormatEntry> Entry,
		const TSharedRef<STableViewBase>& OwnerTable);

	/** Called when a format is selected from the dropdown. */
	void OnListSelectionChanged(TSharedPtr<FFormatEntry> SelectedEntry, ESelectInfo::Type SelectInfo);

	/** Called when the search text changes. */
	void OnSearchTextChanged(const FText& SearchText);

	/** Updates the combo button face to reflect the current value. */
	void RefreshDisplayWidgets();

	// -- Bitmask mode --

	/** Builds the combo button UI for bitmask mode. */
	void BuildBitmaskUI(FDetailWidgetRow& HeaderRow, TSharedRef<IPropertyHandle> PropertyHandle);

	/** Generates the bitmask dropdown menu with checkboxes. */
	TSharedRef<SWidget> GenerateBitmaskMenu();

	/** Reads bitmask value and sets check states on entries. */
	void ReadBitmaskValues();

	/** Writes checked entries back as a bitmask to the property. */
	void WriteBitmaskValues();

	/** Toggles a bitmask entry by index (XOR). */
	void OnToggleBitmaskEntry(int32 Index);

	/** Returns true if the bitmask entry at Index is checked. */
	bool IsBitmaskEntryChecked(int32 Index) const;

	/** Checks all bitmask entries. */
	void OnSelectAllAction();

	/** Unchecks all bitmask entries. */
	void OnClearAllAction();

	/** Returns summary text for bitmask header ("JSON, XML" or "None" or "All"). */
	FText GetBitmaskSummaryText() const;

	/** Updates the bitmask summary text widget. */
	void RefreshBitmaskSummary();

	// -- Members --

	/** The property handle for the FSGSerializerFormat struct. */
	TSharedPtr<IPropertyHandle> StructPropertyHandle = nullptr;

	/** Currently selected format (dropdown mode). */
	FSGSerializerFormat CurrentFormat;

	/** True when SGDTABitmask meta is present (bitmask mode). */
	uint8 bIsBitmaskMode : 1 = 0;

	/** The combo button widget (dropdown mode). */
	TSharedPtr<SComboButton> ComboButton = nullptr;

	/** Display text block on the combo button face (dropdown mode). */
	TSharedPtr<STextBlock> DisplayTextBlock = nullptr;

	/** Icon image on the combo button face (dropdown mode). */
	TSharedPtr<SImage> DisplayIcon = nullptr;

	/** All available format entries. Used by both modes. */
	TArray<TSharedPtr<FFormatEntry>> AllFormatEntries;

	/** Filtered format entries shown in the dropdown list (dropdown mode). */
	TArray<TSharedPtr<FFormatEntry>> FilteredFormatEntries;

	/** The list view widget (dropdown mode, cached for refresh). */
	TSharedPtr<SListView<TSharedPtr<FFormatEntry>>> FormatListView = nullptr;

	/** Summary text block for bitmask header (bitmask mode). */
	TSharedPtr<STextBlock> BitmaskSummaryText = nullptr;
};
