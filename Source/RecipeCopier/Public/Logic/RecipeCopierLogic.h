﻿#pragma once

#include "CommonTypes.h"
#include "FGRecipe.h"
#include "RecipeCopier_ConfigStruct.h"
#include "Buildables/FGBuildableSplitterSmart.h"

#include "RecipeCopierLogic.generated.h"

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
		UPARAM(DisplayName = "Shard Item Descriptor") TSubclassOf<UFGItemDescriptor> in_shardItemDescriptor,
		UPARAM(DisplayName = "Programmable Splitter Class") TSubclassOf<AFGBuildableSplitterSmart> in_programmableSplitterClass
	);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	virtual void Terminate();

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void SetConfiguration(const struct FRecipeCopier_ConfigStruct& in_configuration);

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
		int32 signCopyMode,
		AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);

	UFUNCTION(BlueprintCallable, Category="RecipeCopierLogic")
	static void ApplyTrainInfo
	(
		class AFGTrain* train,
		AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
	);
	static void ApplyTrainInfo_Server
	(
		class AFGTrain* train,
		AFGCharacterPlayer* player,
		class ARecipeCopierEquipment* copier
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

	FCriticalSection eclCritical;

	static ARecipeCopierLogic* singleton;
	static FRecipeCopier_ConfigStruct configuration;
	static TSubclassOf<UFGItemDescriptor> shardItemDescriptor;
	static TSubclassOf<AFGBuildableSplitterSmart> programmableSplitterClass;
};