// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "SquirrelStateCustomization.h"

#include "DetailWidgetRow.h"
#include "Squirrel.h"

#define LOCTEXT_NAMESPACE "SquirrelStateCustomization"

TSharedRef<IPropertyTypeCustomization> FSquirrelStateCustomization::MakeInstance()
{
	return MakeShareable(new FSquirrelStateCustomization);
}

void FSquirrelStateCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	Position = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSquirrelState, Position));
	check(Position)

	const TSharedRef<SWidget> PropertyWidget = Position->CreatePropertyNameWidget();
	PropertyWidget->SetEnabled(PropertyHandle->IsEditable());

	HeaderRow
		.NameContent()
			[
				PropertyWidget
			]
		.ValueContent()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					Position->CreatePropertyValueWidget()
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				[
					SNew(SBox)
					.HAlign(HAlign_Center)
					.VAlign(VAlign_Center)
					.WidthOverride(22)
					.HeightOverride(22)
					.ToolTipText(LOCTEXT("RandomizeButtonTooltip", "Generate a new random position"))
					[
						SNew(SButton)
						.IsEnabled(PropertyHandle->IsEditable())
						.ButtonStyle(FAppStyle::Get(), "SimpleButton")
						.OnClicked(this, &FSquirrelStateCustomization::OnRandomizeClicked)
						.ContentPadding(0)
						.IsFocusable(true)
						[
							SNew(SImage)
							// @todo custom/better icon
							.Image(FAppStyle::GetBrush("Icons.PlusCircle"))
							.ColorAndOpacity(FSlateColor::UseForeground())
						]
					]
				]
			];
}

FReply FSquirrelStateCustomization::OnRandomizeClicked()
{
	check(Position)

	FSquirrelState* DataPtr;
	Position->GetValueData(reinterpret_cast<void*&>(DataPtr));

	if (DataPtr)
	{
		DataPtr->RandomizeState();
	}

	TArray<UObject*> Outers;
	Position->GetOuterObjects(Outers);
	for (UObject* Outer : Outers)
	{
		// Notify the object that is has been modified so that undo/redo works.
		Outer->Modify();
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE