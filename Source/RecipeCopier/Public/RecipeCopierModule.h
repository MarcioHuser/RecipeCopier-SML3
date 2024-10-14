#pragma once

#include "Modules/ModuleManager.h"

#include <map>

#include "RecipeCopier_ConfigStruct.h"
#include "Buildables/FGBuildableFactory.h"

class FRecipeCopierModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;

	static void onPowerCircuitChangedHook(class UFGPowerCircuit* powerCircuit);
	static void setPendingPotentialCallback(class AFGBuildableFactory* buildable, float potential);

	//static std::map<FString, float> powerConsumptionMap;
};
