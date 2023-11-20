#include "RecipeCopierModule.h"
#include "util/Logging.h"
#include "Util/Optimize.h"

#include "UObject/CoreNet.h"

#include "FGCircuitSubsystem.h"
#include "FGPowerCircuit.h"
#include "FGPowerInfoComponent.h"
#include "Buildables/FGBuildableFrackingActivator.h"
#include "Buildables/FGBuildableGeneratorFuel.h"
#include "Patching/NativeHookManager.h"

#ifndef OPTIMIZE
#pragma optimize( "", off )
#endif

// std::map<FString, float> FRecipeCopierModule::powerConsumptionMap;

void FRecipeCopierModule::StartupModule()
{
}

IMPLEMENT_GAME_MODULE(FRecipeCopierModule, RecipeCopier);

#ifndef OPTIMIZE
#pragma optimize( "", on )
#endif
