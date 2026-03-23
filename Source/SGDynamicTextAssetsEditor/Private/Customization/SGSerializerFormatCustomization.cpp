// Copyright Start Games, Inc. All Rights Reserved.

#include "Customization/SGSerializerFormatCustomization.h"

#include "DetailWidgetRow.h"
#include "Management/SGDynamicTextAssetFileManager.h"
#include "Serialization/SGDynamicTextAssetSerializer.h"
#include "Widgets/Images/SImage.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Input/SSearchBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"

#define LOCTEXT_NAMESPACE "SGSerializerFormatCustomization"

TSharedRef<IPropertyTypeCustomization> FSGSerializerFormatCustomization::MakeInstance()
{
	return MakeShareable(new FSGSerializerFormatCustomization());
}

void FSGSerializerFormatCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	StructPropertyHandle = PropertyHandle;

	// Listen for external changes (reset-to-default, undo, etc.)
	StructPropertyHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FSGSerializerFormatCustomization::OnPropertyValueChanged));

	PopulateFormatEntries();

	// Check for bitmask mode
	const FProperty* property = PropertyHandle->GetProperty();
	bIsBitmaskMode = property && property->HasMetaData(TEXT("SGDTABitmask"));

	if (bIsBitmaskMode)
	{
		BuildBitmaskUI(HeaderRow, PropertyHandle);
		return;
	}

	// Dropdown mode
	ReadCurrentValue();

	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(200.0f)
		.MaxDesiredWidth(400.0f)
		[
			SAssignNew(ComboButton, SComboButton)
			.OnGetMenuContent(FOnGetContent::CreateSP(this, &FSGSerializerFormatCustomization::GeneratePickerMenu))
			.ContentPadding(FMargin(4.0f, 2.0f))
			.ButtonContent()
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
				.AutoWidth()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 0.0f, 4.0f, 0.0f)
				[
					SAssignNew(DisplayIcon, SImage)
					.Image(GetCurrentIconBrush())
					.DesiredSizeOverride(FVector2D(16.0f, 16.0f))
				]

				+ SHorizontalBox::Slot()
				.FillWidth(1.0f)
				.VAlign(VAlign_Center)
				[
					SAssignNew(DisplayTextBlock, STextBlock)
					.Text(GetCurrentDisplayText())
					.ToolTipText(GetCurrentTooltipText())
				]
			]
		];
}

void FSGSerializerFormatCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle,
	IDetailChildrenBuilder& ChildBuilder,
	IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	// Hide all children - the header handles everything in both modes
}

void FSGSerializerFormatCustomization::PopulateFormatEntries()
{
	AllFormatEntries.Reset();

	// Add all registered serializer formats (no "None" entry for bitmask mode,
	// but we add it here and skip it in bitmask mode when building UI)
	TArray<TSharedPtr<ISGDynamicTextAssetSerializer>> serializers =
		FSGDynamicTextAssetFileManager::GetAllRegisteredSerializers();

	// Add "None" entry at the top (used by dropdown mode only)
	TSharedPtr<FFormatEntry> noneEntry = MakeShared<FFormatEntry>();
	noneEntry->Format = FSGSerializerFormat();
	noneEntry->Name = INVTEXT("None");
	noneEntry->Extension = TEXT("");
	noneEntry->IconBrush = nullptr;
	AllFormatEntries.Add(noneEntry);

	for (const TSharedPtr<ISGDynamicTextAssetSerializer>& serializer : serializers)
	{
		if (!serializer.IsValid())
		{
			continue;
		}

		TSharedPtr<FFormatEntry> entry = MakeShared<FFormatEntry>();
		entry->Format = serializer->GetSerializerFormat();
		entry->Name = serializer->GetFormatName();
		entry->Extension = serializer->GetFileExtension();
#if WITH_EDITOR
		entry->IconBrush = serializer->GetIconBrush();
#endif
		AllFormatEntries.Add(entry);
	}

	FilteredFormatEntries = AllFormatEntries;
}

void FSGSerializerFormatCustomization::OnPropertyValueChanged()
{
	if (bIsBitmaskMode)
	{
		ReadBitmaskValues();
		RefreshBitmaskSummary();
	}
	else
	{
		ReadCurrentValue();
		RefreshDisplayWidgets();
	}
}

void FSGSerializerFormatCustomization::ReadCurrentValue()
{
	CurrentFormat = FSGSerializerFormat();

	if (!StructPropertyHandle.IsValid())
	{
		return;
	}

	TArray<void*> rawData;
	StructPropertyHandle->AccessRawData(rawData);

	if (!rawData.IsEmpty() && rawData[0] != nullptr)
	{
		CurrentFormat = *static_cast<FSGSerializerFormat*>(rawData[0]);
	}
}

void FSGSerializerFormatCustomization::WriteValue(const FSGSerializerFormat& NewFormat)
{
	if (!StructPropertyHandle.IsValid())
	{
		return;
	}

	StructPropertyHandle->NotifyPreChange();

	TArray<void*> rawData;
	StructPropertyHandle->AccessRawData(rawData);

	if (!rawData.IsEmpty() && rawData[0] != nullptr)
	{
		*static_cast<FSGSerializerFormat*>(rawData[0]) = NewFormat;
	}

	StructPropertyHandle->NotifyPostChange(EPropertyChangeType::ValueSet);
	CurrentFormat = NewFormat;

	RefreshDisplayWidgets();
}

void FSGSerializerFormatCustomization::RefreshDisplayWidgets()
{
	if (DisplayTextBlock.IsValid())
	{
		DisplayTextBlock->SetText(GetCurrentDisplayText());
		DisplayTextBlock->SetToolTipText(GetCurrentTooltipText());
	}
	if (DisplayIcon.IsValid())
	{
		DisplayIcon->SetImage(GetCurrentIconBrush());
	}
}

FText FSGSerializerFormatCustomization::GetCurrentDisplayText() const
{
	if (!CurrentFormat.IsValid())
	{
		return INVTEXT("None");
	}

	FText formatName = CurrentFormat.GetFormatName();
	FString extension = CurrentFormat.GetFileExtension();

	if (!extension.IsEmpty())
	{
		return FText::Format(INVTEXT("{0} ({1})"), formatName, FText::FromString(extension));
	}

	return formatName;
}

FText FSGSerializerFormatCustomization::GetCurrentTooltipText() const
{
	if (!CurrentFormat.IsValid())
	{
		return INVTEXT("No serializer format selected");
	}

	FText formatName = CurrentFormat.GetFormatName();
	FString extension = CurrentFormat.GetFileExtension();
	FText description = CurrentFormat.GetFormatDescription();

	return FText::Format(INVTEXT("{0}\nExtension: {1}\nType ID: {2}\n{3}"),
		formatName,
		FText::FromString(extension),
		FText::FromString(CurrentFormat.ToString()),
		description);
}

const FSlateBrush* FSGSerializerFormatCustomization::GetCurrentIconBrush() const
{
	if (!CurrentFormat.IsValid())
	{
		return nullptr;
	}

	for (const TSharedPtr<FFormatEntry>& entry : AllFormatEntries)
	{
		if (entry->Format == CurrentFormat)
		{
			return entry->IconBrush;
		}
	}

	return nullptr;
}

TSharedRef<SWidget> FSGSerializerFormatCustomization::GeneratePickerMenu()
{
	FilteredFormatEntries = AllFormatEntries;

	TSharedRef<SVerticalBox> menuContent = SNew(SVerticalBox);

	if (AllFormatEntries.Num() >= 10)
	{
		menuContent->AddSlot()
			.AutoHeight()
			.Padding(4.0f)
			[
				SNew(SSearchBox)
				.OnTextChanged(this, &FSGSerializerFormatCustomization::OnSearchTextChanged)
			];
	}

	menuContent->AddSlot()
		.FillHeight(1.0f)
		.MaxHeight(300.0f)
		[
			SNew(SBox)
			.MinDesiredWidth(250.0f)
			[
				SAssignNew(FormatListView, SListView<TSharedPtr<FFormatEntry>>)
				.ListItemsSource(&FilteredFormatEntries)
				.OnGenerateRow(this, &FSGSerializerFormatCustomization::GenerateListRow)
				.OnSelectionChanged(this, &FSGSerializerFormatCustomization::OnListSelectionChanged)
				.SelectionMode(ESelectionMode::Single)
			]
		];

	return menuContent;
}

TSharedRef<ITableRow> FSGSerializerFormatCustomization::GenerateListRow(
	TSharedPtr<FFormatEntry> Entry,
	const TSharedRef<STableViewBase>& OwnerTable)
{
	const bool bIsSelected = Entry.IsValid() && Entry->Format == CurrentFormat;

	return SNew(STableRow<TSharedPtr<FFormatEntry>>, OwnerTable)
		.Padding(FMargin(4.0f, 2.0f))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 0.0f, 6.0f, 0.0f)
			[
				SNew(SImage)
				.Image(Entry->IconBrush)
				.DesiredSizeOverride(FVector2D(16.0f, 16.0f))
				.Visibility(Entry->IconBrush ? EVisibility::Visible : EVisibility::Collapsed)
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(Entry->Name)
				.Font(bIsSelected
					? FAppStyle::GetFontStyle("BoldFont")
					: FAppStyle::GetFontStyle("NormalFont"))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(8.0f, 0.0f, 0.0f, 0.0f)
			[
				SNew(STextBlock)
				.Text(FText::FromString(Entry->Extension))
				.ColorAndOpacity(FSlateColor::UseSubduedForeground())
				.Visibility(Entry->Extension.IsEmpty() ? EVisibility::Collapsed : EVisibility::Visible)
			]
		];
}

void FSGSerializerFormatCustomization::OnListSelectionChanged(
	TSharedPtr<FFormatEntry> SelectedEntry, ESelectInfo::Type SelectInfo)
{
	if (!SelectedEntry.IsValid() || SelectInfo == ESelectInfo::Direct)
	{
		return;
	}

	WriteValue(SelectedEntry->Format);

	if (ComboButton.IsValid())
	{
		ComboButton->SetIsOpen(false);
	}
}

void FSGSerializerFormatCustomization::OnSearchTextChanged(const FText& SearchText)
{
	const FString searchString = SearchText.ToString();

	if (searchString.IsEmpty())
	{
		FilteredFormatEntries = AllFormatEntries;
	}
	else
	{
		FilteredFormatEntries.Reset();
		for (const TSharedPtr<FFormatEntry>& entry : AllFormatEntries)
		{
			if (entry->Name.ToString().Contains(searchString) ||
				entry->Extension.Contains(searchString))
			{
				FilteredFormatEntries.Add(entry);
			}
		}
	}

	if (FormatListView.IsValid())
	{
		FormatListView->RequestListRefresh();
	}
}

void FSGSerializerFormatCustomization::BuildBitmaskUI(
	FDetailWidgetRow& HeaderRow, TSharedRef<IPropertyHandle> PropertyHandle)
{
	ReadBitmaskValues();

	HeaderRow
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		.MinDesiredWidth(200.0f)
		.MaxDesiredWidth(400.0f)
		[
			SAssignNew(ComboButton, SComboButton)
			.OnGetMenuContent(FOnGetContent::CreateSP(this, &FSGSerializerFormatCustomization::GenerateBitmaskMenu))
			.ContentPadding(FMargin(4.0f, 2.0f))
			.ButtonContent()
			[
				SAssignNew(BitmaskSummaryText, STextBlock)
				.Text(GetBitmaskSummaryText())
				.ToolTipText(INVTEXT("Serializer formats included in this selection"))
			]
		];
}

TSharedRef<SWidget> FSGSerializerFormatCustomization::GenerateBitmaskMenu()
{
	// Don't close after selection - allow toggling multiple flags
	FMenuBuilder MenuBuilder(false, nullptr, nullptr, true);

	// Select All / Clear All as standard menu entries
	MenuBuilder.AddMenuEntry(
		INVTEXT("Select All"),
		INVTEXT("Check all serializer formats"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FSGSerializerFormatCustomization::OnSelectAllAction)),
		NAME_None,
		EUserInterfaceActionType::Button);

	MenuBuilder.AddMenuEntry(
		INVTEXT("Clear All"),
		INVTEXT("Uncheck all serializer formats"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateSP(this, &FSGSerializerFormatCustomization::OnClearAllAction)),
		NAME_None,
		EUserInterfaceActionType::Button);

	MenuBuilder.AddSeparator();

	// Format entries with checkmark toggle (skip index 0 which is the "None" entry)
	for (int32 idx = 1; idx < AllFormatEntries.Num(); ++idx)
	{
		const TSharedPtr<FFormatEntry>& entry = AllFormatEntries[idx];
		const int32 entryIndex = idx;

		// Display label includes extension in parentheses
		FText displayLabel = entry->Extension.IsEmpty()
			? entry->Name
			: FText::Format(INVTEXT("{0} ({1})"), entry->Name, FText::FromString(entry->Extension));

		MenuBuilder.AddMenuEntry(
			displayLabel,
			FText(),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &FSGSerializerFormatCustomization::OnToggleBitmaskEntry, entryIndex),
				FCanExecuteAction(),
				FIsActionChecked::CreateSP(this, &FSGSerializerFormatCustomization::IsBitmaskEntryChecked, entryIndex)
			),
			NAME_None,
			EUserInterfaceActionType::Check);
	}

	return MenuBuilder.MakeWidget();
}

void FSGSerializerFormatCustomization::ReadBitmaskValues()
{
	if (!StructPropertyHandle.IsValid())
	{
		return;
	}

	// Reset all check states
	for (TSharedPtr<FFormatEntry>& entry : AllFormatEntries)
	{
		entry->bIsChecked = false;
	}

	TArray<void*> rawData;
	StructPropertyHandle->AccessRawData(rawData);

	if (rawData.IsEmpty() || rawData[0] == nullptr)
	{
		return;
	}

	// Read the uint32 as a bitmask where each bit corresponds to the format's TypeId
	const FSGSerializerFormat* format = static_cast<FSGSerializerFormat*>(rawData[0]);
	const uint32 bitmask = format->GetTypeId();

	// Each format's TypeId is used as the bit position (1 << TypeId)
	for (TSharedPtr<FFormatEntry>& entry : AllFormatEntries)
	{
		if (entry->Format.IsValid())
		{
			const uint32 formatBit = (1u << entry->Format.GetTypeId());
			entry->bIsChecked = (bitmask & formatBit) != 0;
		}
	}
}

void FSGSerializerFormatCustomization::WriteBitmaskValues()
{
	if (!StructPropertyHandle.IsValid())
	{
		return;
	}

	// Build bitmask from checked entries
	uint32 bitmask = 0;
	for (const TSharedPtr<FFormatEntry>& entry : AllFormatEntries)
	{
		if (entry->bIsChecked && entry->Format.IsValid())
		{
			bitmask |= (1u << entry->Format.GetTypeId());
		}
	}

	WriteValue(FSGSerializerFormat(bitmask));
	RefreshBitmaskSummary();
}

void FSGSerializerFormatCustomization::OnToggleBitmaskEntry(int32 Index)
{
	if (Index < 0 || Index >= AllFormatEntries.Num())
	{
		return;
	}

	AllFormatEntries[Index]->bIsChecked = !AllFormatEntries[Index]->bIsChecked;
	WriteBitmaskValues();
}

bool FSGSerializerFormatCustomization::IsBitmaskEntryChecked(int32 Index) const
{
	if (Index >= 0 && Index < AllFormatEntries.Num())
	{
		return AllFormatEntries[Index]->bIsChecked;
	}
	return false;
}

void FSGSerializerFormatCustomization::OnSelectAllAction()
{
	// Skip index 0 (None entry)
	for (int32 idx = 1; idx < AllFormatEntries.Num(); ++idx)
	{
		AllFormatEntries[idx]->bIsChecked = true;
	}
	WriteBitmaskValues();
}

void FSGSerializerFormatCustomization::OnClearAllAction()
{
	for (TSharedPtr<FFormatEntry>& entry : AllFormatEntries)
	{
		entry->bIsChecked = false;
	}
	WriteBitmaskValues();
}

FText FSGSerializerFormatCustomization::GetBitmaskSummaryText() const
{
	int32 checkedCount = 0;
	int32 totalFormats = 0;
	TArray<FString> checkedNames;

	// Skip index 0 (None entry) for counting
	for (int32 idx = 1; idx < AllFormatEntries.Num(); ++idx)
	{
		++totalFormats;
		if (AllFormatEntries[idx]->bIsChecked)
		{
			++checkedCount;
			checkedNames.Add(AllFormatEntries[idx]->Name.ToString());
		}
	}

	if (checkedCount == 0)
	{
		return INVTEXT("None");
	}

	if (checkedCount == totalFormats)
	{
		return INVTEXT("All");
	}

	return FText::FromString(FString::Join(checkedNames, TEXT(" | ")));
}

void FSGSerializerFormatCustomization::RefreshBitmaskSummary()
{
	if (BitmaskSummaryText.IsValid())
	{
		BitmaskSummaryText->SetText(GetBitmaskSummaryText());
	}
}

#undef LOCTEXT_NAMESPACE
