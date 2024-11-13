// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "PropertyEditorModule.h"
#include "Modules/ModuleManager.h"
#include "Squirrel.h"
#include "Customizations/SquirrelStateCustomization.h"

#define LOCTEXT_NAMESPACE "SquirrelEditorModule"

class FSquirrelEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FSquirrelEditorModule::StartupModule()
{
	FPropertyEditorModule& Module = FModuleManager::Get().LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

	Module.RegisterCustomPropertyTypeLayout(FSquirrelState::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FSquirrelStateCustomization::MakeInstance));
}

void FSquirrelEditorModule::ShutdownModule()
{
	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& Module = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		Module.UnregisterCustomPropertyTypeLayout(FSquirrelState::StaticStruct()->GetFName());
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSquirrelEditorModule, SquirrelEditor)