#pragma once
#include "CoreMinimal.h"
#include "Configuration/ConfigManager.h"
#include "Engine/Engine.h"
#include "RecipeCopier_ConfigStruct.generated.h"

/* Struct generated from Mod Configuration Asset '/RecipeCopier/Configuration/RecipeCopier_Config' */
USTRUCT(BlueprintType)
struct FRecipeCopier_ConfigStruct {
    GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    int32 logLevel;

    /* Retrieves active configuration value and returns object of this struct containing it */
    static FRecipeCopier_ConfigStruct GetActiveConfig() {
        FRecipeCopier_ConfigStruct ConfigStruct{};
        FConfigId ConfigId{"RecipeCopier", ""};
        UConfigManager* ConfigManager = GEngine->GetEngineSubsystem<UConfigManager>();
        ConfigManager->FillConfigurationStruct(ConfigId, FDynamicStructInfo{FRecipeCopier_ConfigStruct::StaticStruct(), &ConfigStruct});
        return ConfigStruct;
    }
};

