#include "RecipeCopierModule.h"
#include "Util/RCLogging.h"
#include "Util/RCOptimize.h"

#ifndef OPTIMIZE
#pragma optimize("", off )
#endif

// std::map<FString, float> FRecipeCopierModule::powerConsumptionMap;

void FRecipeCopierModule::StartupModule()
{
}

IMPLEMENT_GAME_MODULE(FRecipeCopierModule, RecipeCopier);

#ifndef OPTIMIZE
#pragma optimize("", on )
#endif
