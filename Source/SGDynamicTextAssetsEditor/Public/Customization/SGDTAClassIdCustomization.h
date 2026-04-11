// Copyright Start Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "Core/SGDTAClassId.h"
#include "Input/Reply.h"
#include "IPropertyTypeCustomization.h"

class SButton;
class SComboButton;

/**
 * Property type customization for FSGDTAClassId.
 *
 * Replaces the default struct view with a class picker and GUID controls:
 * +----------------------------------------------------------------------+
 * | [Class Picker Dropdown          v] [Copy] [Paste] [X]               |
 * +----------------------------------------------------------------------+
 *
 * Features:
 * - Class picker dropdown filtered by meta=(SGDTAClassType="BaseClassName")
 * - Copy button copies Class ID GUID to clipboard
 * - Paste button reads GUID from clipboard and sets value
 * - Clear button resets to INVALID_CLASS_ID
 * - Class picker fully wired once extender registry exists
 */
class FSGDTAClassIdCustomization : public IPropertyTypeCustomization
{
public:

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

	/** Returns the display text for the currently selected class ID */
	FText GetCurrentDisplayText() const;

	/** Returns the tooltip for the currently selected class ID */
	FText GetCurrentTooltipText() const;

	/** Called when the class picker selects a class (or nullptr for None) */
	void OnClassPicked(UClass* NewClass);

	/** Called when the Clear button is clicked */
	FReply OnClearClicked();

	/** Called when the Copy ID button is clicked */
	FReply OnCopyIdClicked();

	/** Called when the Paste ID button is clicked */
	FReply OnPasteClicked();

	/** Reads the current FSGDTAClassId value from the property handle */
	void ReadCurrentValue();

	/** Writes an updated FSGDTAClassId back to the property handle */
	void WriteValue(const FSGDTAClassId& NewClassId);

	/** Updates button enabled states from current values */
	void RefreshButtonStates();

	/** The property handle for the FSGDTAClassId struct */
	TSharedPtr<IPropertyHandle> StructPropertyHandle = nullptr;

	/** Currently selected class ID */
	FSGDTAClassId CurrentClassId;

	/** Base class filter resolved from meta tag */
	TObjectPtr<UClass> FilterBaseClass = nullptr;

	/** The class picker combo button */
	TSharedPtr<SComboButton> ClassPickerCombo = nullptr;

	/** Copy ID button reference for event-based enable/disable control */
	TSharedPtr<SButton> CopyIdButton = nullptr;

	/** Clear button reference for event-based enable/disable control */
	TSharedPtr<SButton> ClearButton = nullptr;
};
