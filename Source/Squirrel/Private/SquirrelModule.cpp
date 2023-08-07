// Copyright Guy (Drakynfly) Lundvall. All Rights Reserved.

#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "SquirrelModule"

class FSquirrelModule final : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FSquirrelModule::StartupModule()
{
}

void FSquirrelModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSquirrelModule, Squirrel)