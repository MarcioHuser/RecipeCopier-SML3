#pragma once

#include "CommonTypes.h"
#include "FGRailroadTimeTable.h"
#include "SignCopyModeType.h"
#include "Buildables/FGBuildableLightSource.h"
#include "Components/WidgetComponent.h"
#include "Equipment/FGEquipment.h"

#include "RecipeCopierEquipment.generated.h"

UCLASS(BlueprintType)
class RECIPECOPIER_API ARecipeCopierEquipment : public AFGEquipment
{
	GENERATED_BODY()
public:
	ARecipeCopierEquipment();

	virtual void BeginPlay() override;

	virtual void HandleDefaultEquipmentActionEvent( EDefaultEquipmentAction action, EDefaultEquipmentActionEvent actionEvent ) override;
	
	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void HandleHitActor(AActor* hitActor, bool& wasHit);

	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void PrimaryFirePressed();

	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void CopyTarget();

	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void CopyFactory();
	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void CopySmartSplitter();
	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void CopyWidgetSign();
	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void CopyTrain();
	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void CopyLightsControlPanel();
	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void CopyValve();

	UFUNCTION(BlueprintCallable, Category = "RecipeCopier", NetMulticast, Reliable)
	virtual void ClearTargets();

	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void ApplyTarget();

	UFUNCTION(BlueprintImplementableEvent, Category = "RecipeCopier")
	void SetTextureWidget(UWidgetComponent* widgetComponent);

	UFUNCTION(BlueprintImplementableEvent, Category = "RecipeCopier")
	void SetWidgetSmartSplitterInfo(const TArray<struct FSplitterSortRule>& currentSplitterRules, const TArray<struct FSplitterSortRule>& savedSplitterRules);

	UFUNCTION(BlueprintImplementableEvent, Category = "RecipeCopier")
	void SetWidgetFactoryInfo(TSubclassOf<class UFGRecipe> currentRecipe, TSubclassOf<class UFGRecipe> savedRecipe, float currentOverclock, float savedOverclock);

	UFUNCTION(BlueprintImplementableEvent, Category = "RecipeCopier")
	void SetWidgetSignInfo
	(
		bool currentIsDefined,
		bool savedIsDefined,
		const FLinearColor& currentForegroundColor,
		const FLinearColor& savedForegroundColor,
		const FLinearColor& currentBackgroundColor,
		const FLinearColor& savedBackgroundColor,
		const FLinearColor& currentAuxiliaryColor,
		const FLinearColor& savedAuxiliaryColor,
		float currentEmissive,
		float savedEmissive,
		float currentGlossiness,
		float savedGlossiness,
		const TMap<FString, FString>& currentTexts,
		const TMap<FString, FString>& savedTexts,
		const TMap<FString, int32>& currentIconIDs,
		const TMap<FString, int32>& savedIconIDs,
		const FText& currentPrefabLayoutDescription,
		const FText& savedPrefabLayoutDescription,
		UPARAM(DisplayName = "Sign Copy Mode", meta = (Bitmask, BitmaskEnum = ESignCopyModeType)) int32 in_signCopyMode
	);

	UFUNCTION(BlueprintImplementableEvent, Category = "RecipeCopier")
	void SetWidgetTrainInfo
	(
		const TArray<FTimeTableStop>& currentTrainStops,
		const TArray<FTimeTableStop>& savedTrainStops
	);

	UFUNCTION(BlueprintImplementableEvent, Category = "RecipeCopier")
	void SetWidgetLightsControlPanelInfo
	(
		const FLightSourceControlData& currentLightSourceControlData,
		const FLightSourceControlData& savedLightSourceControlData,
		bool currentIsLightEnabled,
		bool savedIsLightEnabled
	);

	UFUNCTION(BlueprintImplementableEvent, Category = "RecipeCopier")
	void SetWidgetValveInfo
	(
		float currentUserFlowLimit,
		float savedUserFlowLimit
	);

	UFUNCTION(BlueprintImplementableEvent, Category="RecipeCopier")
	void PlayObjectScannerCycleRightAnim();

	UFUNCTION(BlueprintCallable, Category = "RecipeCopier")
	virtual void CycleCopyMode();

	UFUNCTION(BlueprintCallable, Category="RecipeCopier")
	virtual void SetTargets
	(
		class AFGBuildableSplitterSmart* smartSplitter,
		class AFGTrain* train,
		class AFGBuildableWidgetSign* widgetSign,
		class AFGBuildableLightsControlPanel* lightsControlPanel,
		class AFGBuildablePipelinePump* valve,
		class AFGBuildableFactory* factory
	);

	bool CompareSplitterRules(class AFGBuildableSplitterSmart* smartSplitter);

	virtual void HandleAimSmartSplitter(class AFGCharacterPlayer* character, class AFGBuildableSplitterSmart* smartSplitter);
	virtual void HandleAimFactory(class AFGPlayerController* playerController, class AFGCharacterPlayer* character, class AFGBuildableFactory* factory);
	virtual void HandleAimSign(class AFGCharacterPlayer* character, class AFGBuildableWidgetSign* widgetSign);
	virtual void HandleAimTrain(class AFGCharacterPlayer* character, class AFGTrain* train);
	virtual void HandleAimLightsControlPanel(class AFGCharacterPlayer* character, class AFGBuildableLightsControlPanel* lightsControlPanel);
	virtual void HandleAimValve(class AFGCharacterPlayer* character, class AFGBuildablePipelinePump* valve);

	static FString GetAuthorityAndPlayer(const AActor* actor);

	FString _TAG_NAME = TEXT("RecipeCopierEquipment: ");

	// inline static FString
	// getTimeStamp()
	// {
	// 	const auto now = FDateTime::Now();
	//
	// 	return FString::Printf(TEXT("%02d:%02d:%02d"), now.GetHour(), now.GetMinute(), now.GetSecond());
	// }

	inline FString
	getTagName() const
	{
		return /*getTimeStamp() + TEXT(" ") +*/ _TAG_NAME;
	}

protected:
	UPROPERTY(BlueprintReadWrite)
	class AFGBuildableFactory* targetFactory = nullptr;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UFGRecipe> selectedRecipe;
	UPROPERTY(BlueprintReadWrite)
	float selectedOverclock = 0;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<UFGRecipe> aimedRecipe;
	UPROPERTY(BlueprintReadWrite)
	float aimedOverclock = 0;
	UPROPERTY(BlueprintReadWrite)
	ERecipeCopyMode recipeCopyMode = ERecipeCopyMode::RecipeAndOverclock;

	UPROPERTY(BlueprintReadWrite)
	class AFGBuildableSplitterSmart* targetSmartSplitter = nullptr;
	UPROPERTY(BlueprintReadWrite)
	TArray<FSplitterSortRule> selectedSplitterRules;
	UPROPERTY(BlueprintReadWrite)
	TArray<FSplitterSortRule> aimedSplitterRules;

	UPROPERTY(BlueprintReadWrite)
	class AFGTrain* targetTrain = nullptr;
	UPROPERTY(BlueprintReadWrite)
	TArray<FTimeTableStop> aimedTrainStops;
	UPROPERTY(BlueprintReadWrite)
	TArray<FTimeTableStop> selectedTrainStops;

	UPROPERTY(BlueprintReadWrite)
	class AFGBuildableWidgetSign* targetWidgetSign = nullptr;
	UPROPERTY(BlueprintReadWrite)
	bool selectedIsDefined = false;
	UPROPERTY(BlueprintReadWrite)
	FLinearColor selectedForegroundColor;
	UPROPERTY(BlueprintReadWrite)
	FLinearColor selectedBackgroundColor;
	UPROPERTY(BlueprintReadWrite)
	FLinearColor selectedAuxiliaryColor;
	UPROPERTY(BlueprintReadWrite)
	float selectedEmissive = 0;
	UPROPERTY(BlueprintReadWrite)
	float selectedGlossiness = 0;
	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FString> selectedTexts;
	UPROPERTY(BlueprintReadWrite)
	TMap<FString, int32> selectedIconIDs;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UFGSignPrefabWidget> selectedPrefabLayout = nullptr;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UFGSignTypeDescriptor> selectedSignTypeDesc = nullptr;
	UPROPERTY(BlueprintReadWrite)
	bool aimedIsDefined = false;
	UPROPERTY(BlueprintReadWrite)
	FLinearColor aimedForegroundColor;
	UPROPERTY(BlueprintReadWrite)
	FLinearColor aimedBackgroundColor;
	UPROPERTY(BlueprintReadWrite)
	FLinearColor aimedAuxiliaryColor;
	UPROPERTY(BlueprintReadWrite)
	float aimedEmissive = 0;
	UPROPERTY(BlueprintReadWrite)
	float aimedGlossiness = 0;
	UPROPERTY(BlueprintReadWrite)
	TMap<FString, FString> aimedTexts;
	UPROPERTY(BlueprintReadWrite)
	TMap<FString, int32> aimedIconIDs;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UFGSignPrefabWidget> aimedPrefabLayout = nullptr;
	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<class UFGSignTypeDescriptor> aimedSignTypeDesc = nullptr;
	UPROPERTY(BlueprintReadWrite, meta = (Bitmask, BitmaskEnum = ESignCopyModeType))
	int32 signCopyMode = TO_ESignCopyModeType(ESignCopyModeType::SCMT_All); // No layout by default

	UPROPERTY(BlueprintReadWrite)
	class AFGBuildableLightsControlPanel* targetLightsControlPanel = nullptr;
	UPROPERTY(BlueprintReadWrite)
	FLightSourceControlData aimedLightSourceControlData;
	UPROPERTY(BlueprintReadWrite)
	bool aimedIsLightEnabled = false;
	UPROPERTY(BlueprintReadWrite)
	FLightSourceControlData selectedLightSourceControlData;
	UPROPERTY(BlueprintReadWrite)
	bool selectedIsLightEnabled = false;

	UPROPERTY(BlueprintReadWrite)
	class AFGBuildablePipelinePump* targetValve = nullptr;
	UPROPERTY(BlueprintReadWrite)
	float aimedUserFlowLimit = 0;
	UPROPERTY(BlueprintReadWrite)
	float selectedUserFlowLimit = 0;
	

	UPROPERTY(BlueprintReadWrite)
	FKey toggleKey = EKeys::RightMouseButton;

	UPROPERTY(BlueprintReadWrite)
	UWidgetComponent* widgetFactoryInfo = nullptr;
	UPROPERTY(BlueprintReadWrite)
	UWidgetComponent* widgetSmartSplitterInfo = nullptr;
	UPROPERTY(BlueprintReadWrite)
	UWidgetComponent* widgetTrainInfo = nullptr;
	UPROPERTY(BlueprintReadWrite)
	UWidgetComponent* widgetSignInfo = nullptr;
	UPROPERTY(BlueprintReadWrite)
	UWidgetComponent* widgetLightsControlPanelInfo = nullptr;
	UPROPERTY(BlueprintReadWrite)
	UWidgetComponent* widgetValveInfo = nullptr;

	UWidgetComponent* currentWidgetInfo = nullptr;

	UPROPERTY(BlueprintReadWrite)
	UPointLightComponent* pointLight = nullptr;

public:
	// FORCEINLINE ~ARecipeCopierEquipment() = default;
};
