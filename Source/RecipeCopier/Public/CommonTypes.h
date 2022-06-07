#pragma once

#include "CommonTypes.generated.h"

UENUM(BlueprintType)
enum class ERecipeCopyMode : uint8
{
	RecipeOnly UMETA(DisplayName = "Copy recipe only"),
	OverclockOnly UMETA(DisplayName = "Copy overclock only"),
	RecipeAndOverclock UMETA(DisplayName = "Copy recipe and overclock")
};
