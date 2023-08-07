// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "SquirrelStateCustomization.h"

#include "DetailWidgetRow.h"
#include "Squirrel.h"

TSharedRef<IPropertyTypeCustomization> FSquirrelStateCustomization::MakeInstance()
{
	return MakeShareable(new FSquirrelStateCustomization);
}

void FSquirrelStateCustomization::CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle,
	FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	const TSharedPtr<IPropertyHandle> Position = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FSquirrelState, Position));
	check(Position)

	HeaderRow
		.NameContent()
			[
				Position->CreatePropertyNameWidget()
			]
		.ValueContent()
			[
				Position->CreatePropertyValueWidget()
			];
}
