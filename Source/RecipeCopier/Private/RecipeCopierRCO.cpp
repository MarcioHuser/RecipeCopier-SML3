#include "RecipeCopierRCO.h"
#include "RecipeCopierEquipment.h"
#include "Logic/RecipeCopierLogic.h"
#include "Util/RCOptimize.h"
#include "Buildables/FGBuildableFactory.h"
#include "FGPlayerController.h"
#include "FGSignTypes.h"
#include "FGTrain.h"
#include "Buildables/FGBuildableLightsControlPanel.h"
#include "Buildables/FGBuildablePipelinePump.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "Net/UnrealNetwork.h"
#include "Util/MapHelpers.h"

#ifndef OPTIMIZE
#pragma optimize("", off )
#endif

void URecipeCopierRCO::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(URecipeCopierRCO, dummy);
}

URecipeCopierRCO* URecipeCopierRCO::getRCO(UWorld* world)
{
	return Cast<URecipeCopierRCO>(
		Cast<AFGPlayerController>(world->GetFirstPlayerController())->GetRemoteCallObjectOfClass(URecipeCopierRCO::StaticClass())
	);
}

void URecipeCopierRCO::ApplyFactoryInfo_Implementation
(
	class AFGBuildableFactory* factory,
	TSubclassOf<UFGRecipe> recipe,
	float overclock,
	float productionBoost,
	ERecipeCopyMode copyMode,
	class AFGCharacterPlayer* player,
	class ARecipeCopierEquipment* copier
)
{
	if (IsValid(factory) && factory->HasAuthority())
	{
		ARecipeCopierLogic::ApplyFactoryInfo_Server(
			factory,
			recipe,
			overclock,
			productionBoost,
			copyMode,
			player,
			copier
		);
	}
}

bool URecipeCopierRCO::ApplyFactoryInfo_Validate
(
	class AFGBuildableFactory* factory,
	TSubclassOf<UFGRecipe> recipe,
	float selectedOverclock,
	float selectedProductionBoost,
	ERecipeCopyMode copyMode,
	class AFGCharacterPlayer* player,
	class ARecipeCopierEquipment* copier
)
{
	return IsValid(factory);
}

void URecipeCopierRCO::MoveItems_Implementation
(
	UFGInventoryComponent* sourceInventoryComponent,
	bool takeFromCentralStorage,
	UFGInventoryComponent* targetInventoryComponent,
	TSubclassOf<UFGItemDescriptor> item,
	int amount,
	bool dropExcess,
	AFGCharacterPlayer* player
)
{
	if (IsValid(sourceInventoryComponent) && IsValid(targetInventoryComponent) && sourceInventoryComponent->HasAuthority())
	{
		ARecipeCopierLogic::MoveItems_Server(
			sourceInventoryComponent,
			takeFromCentralStorage,
			targetInventoryComponent,
			item,
			amount,
			dropExcess,
			player
		);
	}
}

bool URecipeCopierRCO::MoveItems_Validate
(
	UFGInventoryComponent* sourceInventoryComponent,
	bool takeFromCentralStorage,
	UFGInventoryComponent* targetInventoryComponent,
	TSubclassOf<UFGItemDescriptor> item,
	int amount,
	bool dropExcess,
	AFGCharacterPlayer* player
)
{
	return IsValid(sourceInventoryComponent) && IsValid(targetInventoryComponent);
}

void URecipeCopierRCO::ApplySmartSplitterInfo_Implementation
(
	class AFGBuildableSplitterSmart* smartSplitter,
	const TArray<FSplitterSortRule>& splitterRules,
	class AFGCharacterPlayer* player,
	class ARecipeCopierEquipment* copier
)
{
	if (IsValid(smartSplitter) && smartSplitter->HasAuthority())
	{
		ARecipeCopierLogic::ApplySmartSplitterInfo_Server(
			smartSplitter,
			splitterRules,
			player,
			copier
		);
	}
}

bool URecipeCopierRCO::ApplySmartSplitterInfo_Validate
(
	class AFGBuildableSplitterSmart* smartSplitter,
	const TArray<FSplitterSortRule>& splitterRules,
	class AFGCharacterPlayer* player,
	class ARecipeCopierEquipment* copier
)
{
	return IsValid(smartSplitter);
}

void URecipeCopierRCO::ApplyTrainInfo_Implementation
(
	AFGTrain* train,
	const TArray<FTimeTableStop>& trainStops,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (IsValid(train) && train->HasAuthority())
	{
		ARecipeCopierLogic::ApplyTrainInfo_Server(
			train,
			trainStops,
			player,
			copier
		);
	}
}

bool URecipeCopierRCO::ApplyTrainInfo_Validate
(
	AFGTrain* train,
	const TArray<FTimeTableStop>& trainStops,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	return IsValid(train);
}

void URecipeCopierRCO::ApplyWidgetSignInfo_Implementation
(
	AFGBuildableWidgetSign* widgetSign,
	const FLinearColor& foregroundColor,
	const FLinearColor& backgroundColor,
	const FLinearColor& auxiliaryColor,
	float emissive,
	float glossiness,
	const TArray<FString>& textKeys,
	const TArray<FString>& textValues,
	const TArray<FString>& iconIdKeys,
	const TArray<int32>& iconIdValues,
	TSubclassOf<class UFGSignPrefabWidget> prefabLayout,
	TSubclassOf<class UFGSignTypeDescriptor> signTypeDesc,
	int32 signCopyMode,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (IsValid(widgetSign) && widgetSign->HasAuthority())
	{
		TMap<FString, FString> texts;
		TMap<FString, int32> iconIds;

		ArraysToMap(textKeys, textValues, texts);
		ArraysToMap(iconIdKeys, iconIdValues, iconIds);

		ARecipeCopierLogic::ApplyWidgetSignInfo_Server(
			widgetSign,
			foregroundColor,
			backgroundColor,
			auxiliaryColor,
			emissive,
			glossiness,
			texts,
			iconIds,
			*prefabLayout,
			signTypeDesc,
			signCopyMode,
			player,
			copier
		);
	}
}

bool URecipeCopierRCO::ApplyWidgetSignInfo_Validate
(
	AFGBuildableWidgetSign* widgetSign,
	const FLinearColor& foregroundColor,
	const FLinearColor& backgroundColor,
	const FLinearColor& auxiliaryColor,
	float emissive,
	float glossiness,
	const TArray<FString>& textKeys,
	const TArray<FString>& textValues,
	const TArray<FString>& iconIdKeys,
	const TArray<int32>& iconIdValues,
	TSubclassOf<class UFGSignPrefabWidget> prefabLayout,
	TSubclassOf<class UFGSignTypeDescriptor> signTypeDesc,
	int32 signCopyMode,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	return IsValid(widgetSign);
}

void URecipeCopierRCO::ApplyLightsControlPanel_Implementation
(
	AFGBuildableLightsControlPanel* lightsControlPanel,
	const FLightSourceControlData& lightSourceControlData,
	bool isLightEnabled,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (IsValid(lightsControlPanel) && lightsControlPanel->HasAuthority())
	{
		ARecipeCopierLogic::ApplyLightsControlPanel_Server(
			lightsControlPanel,
			lightSourceControlData,
			isLightEnabled,
			player,
			copier
		);
	}
}

bool URecipeCopierRCO::ApplyLightsControlPanel_Validate
(
	AFGBuildableLightsControlPanel* lightsControlPanel,
	const FLightSourceControlData& lightSourceControlData,
	bool isLightEnabled,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	return IsValid(lightsControlPanel) ;
}

void URecipeCopierRCO::ApplyValve_Implementation
(
	AFGBuildablePipelinePump* valve,
	float userFlowLimit,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (IsValid(valve) && valve->HasAuthority())
	{
		ARecipeCopierLogic::ApplyValve_Server(
			valve,
			userFlowLimit,
			player,
			copier
		);
	}
}

bool URecipeCopierRCO::ApplyValve_Validate
(
	AFGBuildablePipelinePump* valve,
	float userFlowLimit,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	return IsValid(valve);
}

#ifndef OPTIMIZE
#pragma optimize("", on )
#endif
