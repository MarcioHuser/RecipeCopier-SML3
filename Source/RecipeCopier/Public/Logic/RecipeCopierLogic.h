#pragma once

#include "CommonTypes.h"
#include "FGRailroadTimeTable.h"
#include "FGRecipe.h"
#include "RecipeCopier_ConfigStruct.h"
#include "Buildables/FGBuildableLightSource.h"
#include "Buildables/FGBuildableSplitterSmart.h"

#include "RecipeCopierLogic.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FAppendLabel, class UPanelWidget*, container, const FString&, value);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FAppendItemIcon, class UPanelWidget*, container, TSubclassOf<UFGItemDescriptor>, itemDesc);

UCLASS()
class RECIPECOPIER_API ARecipeCopierLogic : public AActor
{
	GENERATED_BODY()

public:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category="RecipeCopierLogic")
	static int GetLogLevelRC();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "RecipeCopierLogic")
	static bool IsLogLevelDisplayRC();

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	virtual void Initialize
	(
		UPARAM(DisplayName = "Shard Item Descriptor") TSubclassOf<class UFGItemDescriptor> in_shardItemDescriptor,
		UPARAM(DisplayName = "Programmable Splitter Class") TSubclassOf<class AFGBuildableSplitterSmart> in_programmableSplitterClass,
		UPARAM(DisplayName = "Valve Class") TSubclassOf<class AFGBuildablePipelinePump> in_valveClass
	);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	virtual void Terminate();

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void SetConfiguration(const struct FRecipeCopier_ConfigStruct& in_configuration);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void DumpUnknownClass(UObject* obj);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void ApplyFactoryInfo
	(
		class AFGBuildableFactory* factory,
		const TSubclassOf<UFGRecipe>& recipe,
		float overclock,
		ERecipeCopyMode copyMode,
		class AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);
	static void ApplyFactoryInfo_Server
	(
		class AFGBuildableFactory* factory,
		const TSubclassOf<UFGRecipe>& recipe,
		float overclock,
		ERecipeCopyMode copyMode,
		class AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void ApplySmartSplitterInfo
	(
		class AFGBuildableSplitterSmart* smartSplitter,
		const TArray<FSplitterSortRule> splitterRules,
		AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);
	static void ApplySmartSplitterInfo_Server
	(
		class AFGBuildableSplitterSmart* smartSplitter,
		const TArray<FSplitterSortRule> splitterRules,
		AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void ApplyWidgetSignInfo
	(
		class AFGBuildableWidgetSign* widgetSign,
		const FLinearColor& foregroundColor,
		const FLinearColor& backgroundColor,
		const FLinearColor& auxiliaryColor,
		float emissive,
		float glossiness,
		const TMap<FString, FString>& texts,
		const TMap<FString, int32>& iconIDs,
		TSubclassOf<class UFGSignPrefabWidget> prefabLayout,
		TSubclassOf<class UFGSignTypeDescriptor> signTypeDesc,
		int32 signCopyMode,
		AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);
	static void ApplyWidgetSignInfo_Server
	(
		class AFGBuildableWidgetSign* widgetSign,
		const FLinearColor& foregroundColor,
		const FLinearColor& backgroundColor,
		const FLinearColor& auxiliaryColor,
		float emissive,
		float glossiness,
		const TMap<FString, FString>& texts,
		const TMap<FString, int32>& iconIDs,
		TSubclassOf<class UFGSignPrefabWidget> prefabLayout,
		TSubclassOf<class UFGSignTypeDescriptor> signTypeDesc,
		int32 signCopyMode,
		AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void ApplyTrainInfo
	(
		class AFGTrain* train,
		const TArray<FTimeTableStop>& trainStops,
		AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);
	static void ApplyTrainInfo_Server
	(
		class AFGTrain* train,
		const TArray<FTimeTableStop>& trainStops,
		AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void ApplyLightsControlPanel
	(
		class AFGBuildableLightsControlPanel* lightsControlPanel,
		const FLightSourceControlData& lightSourceControlData,
		bool isLightEnabled,
		AFGCharacterPlayer* player,
		ARecipeCopierEquipment* copier
	);
	static void ApplyLightsControlPanel_Server
	(
		class AFGBuildableLightsControlPanel* lightsControlPanel,
		const FLightSourceControlData& lightSourceControlData,
		bool isLightEnabled,
		AFGCharacterPlayer* player,
		ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void ApplyValve
	(
		class AFGBuildablePipelinePump* valve,
		float userFlowLimit,
		AFGCharacterPlayer* player,
		ARecipeCopierEquipment* copier
	);
	static void ApplyValve_Server
	(
		class AFGBuildablePipelinePump* valve,
		float userFlowLimit,
		AFGCharacterPlayer* player,
		ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static bool CanProduceRecipe(class AFGBuildableManufacturer* manufacturer, TSubclassOf<UFGRecipe> recipe);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void MoveItems
	(
		class UFGInventoryComponent* sourceInventoryComponent,
		class UFGInventoryComponent* targetInventoryComponent,
		TSubclassOf<UFGItemDescriptor> item,
		int amount,
		bool dropExcess,
		AFGCharacterPlayer* player
	);
	static void MoveItems_Server
	(
		class UFGInventoryComponent* sourceInventoryComponent,
		class UFGInventoryComponent* targetInventoryComponent,
		TSubclassOf<UFGItemDescriptor> item,
		int amount,
		bool dropExcess,
		AFGCharacterPlayer* player
	);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void ConcatTexts(const TMap<FString, FString>& texts, FString& result, const FString& connector = TEXT(", "));

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void SetTimetable
	(
		UPanelWidget* containerWidget,
		FAppendLabel appendLabelCallback,
		FAppendItemIcon appendItemIconCallback,
		const TArray<FTimeTableStop>& trainStops
	);

	FCriticalSection eclCritical;

	static ARecipeCopierLogic* singleton;
	static FRecipeCopier_ConfigStruct configuration;
	static TSubclassOf<UFGItemDescriptor> shardItemDescriptor;
	static TSubclassOf<AFGBuildableSplitterSmart> programmableSplitterClass;
	static TSubclassOf<AFGBuildablePipelinePump> valveClass;
};
