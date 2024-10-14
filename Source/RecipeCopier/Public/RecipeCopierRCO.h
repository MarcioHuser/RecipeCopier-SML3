#pragma once

#include "FGRecipe.h"
#include "FGRemoteCallObject.h"
#include "CommonTypes.h"
#include "FGRailroadTimeTable.h"
#include "Buildables/FGBuildableLightSource.h"
#include "Buildables/FGBuildableSplitterSmart.h"
#include "Resources/FGEquipmentDescriptor.h"
#include "RecipeCopierRCO.generated.h"

UCLASS(Blueprintable)
class RECIPECOPIER_API URecipeCopierRCO : public UFGRemoteCallObject
{
	GENERATED_BODY()
public:
	static URecipeCopierRCO* getRCO(UWorld* world);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierRCO", DisplayName="Get Power Checker RCO")
	static URecipeCopierRCO*
	getRCO(AActor* actor)
	{
		return getRCO(actor->GetWorld());
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="RecipeCopierRCO")
	virtual void ApplyFactoryInfo
	(
		class AFGBuildableFactory* factory,
		TSubclassOf<UFGRecipe> recipe,
		float overclock,
		float productionBoost,
		ERecipeCopyMode copyMode,
		class AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="RecipeCopierRCO")
	virtual void MoveItems
	(
		class UFGInventoryComponent* sourceInventoryComponent,
		bool takeFromCentralStorage,
		class UFGInventoryComponent* targetInventoryComponent,
		TSubclassOf<UFGItemDescriptor> item,
		int amount,
		bool dropExcess,
		AFGCharacterPlayer* player
	);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="RecipeCopierRCO")
	virtual void ApplySmartSplitterInfo
	(
		class AFGBuildableSplitterSmart* smartSplitter,
		const TArray<FSplitterSortRule>& splitterRules,
		class AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="RecipeCopierRCO")
	virtual void ApplyWidgetSignInfo
	(
		class AFGBuildableWidgetSign* widgetSign,
		const FLinearColor& foregroundColor,
		const FLinearColor& backgroundColor,
		const FLinearColor& auxiliaryColor,
		float emissive,
		float glossiness,
		const TArray<FString>& textKeys,
		const TArray<FString>& textValues,
		const TArray<FString>& iconIdKeys,
		const TArray<int32>& iconIdValues,
		// TSoftClassPtr<class UFGSignPrefabWidget> prefabLayout,
		TSubclassOf<class UFGSignTypeDescriptor> signTypeDesc,
		int32 signCopyMode,
		AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="RecipeCopierRCO")
	void ApplyTrainInfo
	(
		class AFGTrain* train,
		const TArray<FTimeTableStop>& trainStops,
		AFGCharacterPlayer* player,
		ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="RecipeCopierRCO")
	void ApplyLightsControlPanel
	(
		class AFGBuildableLightsControlPanel* lightsControlPanel,
		const FLightSourceControlData& lightSourceControlData,
		bool isLightEnabled,
		AFGCharacterPlayer* player,
		ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Server, WithValidation, Reliable, Category="RecipeCopierRCO")
	void ApplyValve
	(
		class AFGBuildablePipelinePump* valve,
		float userFlowLimit,
		AFGCharacterPlayer* player,
		ARecipeCopierEquipment* copier
	);

	UPROPERTY(Replicated)
	bool dummy = true;
};
