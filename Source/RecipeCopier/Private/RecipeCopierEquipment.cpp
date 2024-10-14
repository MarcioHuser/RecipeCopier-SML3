#include "RecipeCopierEquipment.h"
#include "RecipeCopierModule.h"
#include "RecipeCopierRCO.h"
#include "Logic/RecipeCopierLogic.h"
#include "Util/RCOptimize.h"
#include "Util/RCLogging.h"

#include <map>

#include "FGIconLibrary.h"
#include "FGLocomotive.h"
#include "FGPlayerController.h"
#include "FGRailroadTimeTable.h"
#include "FGTrain.h"
#include "Buildables/FGBuildableLightsControlPanel.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildablePipelinePump.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "Components/WidgetComponent.h"
#include "Util/MarcioCommonLibsUtils.h"

#ifndef OPTIMIZE
#pragma optimize("", off )
#endif

template <typename K, typename V>
bool compareMaps(const TMap<K, V>& a, const TMap<K, V>& b)
{
	if (a.Num() != b.Num())
	{
		return false;
	}

	for (auto t : a)
	{
		if (!b.Contains(t.Key) || t.Value != b[t.Key])
		{
			return false;
		}
	}

	return true;
}

template <typename K, typename V>
TArray<V> mapValues(const TMap<K, V>& m)
{
	TArray<V> values;

	for (auto t : m)
	{
		values.Add(t.Value);
	}

	return values;
}

template <typename T>
bool compareArrays(const TArray<T>& a, const TArray<T>& b)
{
	if (a.Num() != b.Num())
	{
		return false;
	}

	for (auto x = 0; x < a.Num(); x++)
	{
		if (a[x] != b[x])
		{
			return false;
		}
	}

	return true;
}

template <>
bool compareArrays<FTimeTableStop>(const TArray<FTimeTableStop>& a, const TArray<FTimeTableStop>& b)
{
	if (a.Num() != b.Num())
	{
		return false;
	}

	for (auto x = 0; x < a.Num(); x++)
	{
		if (a[x].Station != b[x].Station)
		{
			return false;
		}

		if (a[x].DockingRuleSet.DockingDefinition != b[x].DockingRuleSet.DockingDefinition)
		{
			return false;
		}

		if (a[x].DockingRuleSet.IsDurationAndRule != b[x].DockingRuleSet.IsDurationAndRule)
		{
			return false;
		}

		if (a[x].DockingRuleSet.DockForDuration != b[x].DockingRuleSet.DockForDuration)
		{
			return false;
		}

		if (!compareArrays(a[x].DockingRuleSet.LoadFilterDescriptors, b[x].DockingRuleSet.LoadFilterDescriptors))
		{
			return false;
		}

		if (!compareArrays(a[x].DockingRuleSet.UnloadFilterDescriptors, b[x].DockingRuleSet.UnloadFilterDescriptors))
		{
			return false;
		}
	}

	return true;
}

ARecipeCopierEquipment::ARecipeCopierEquipment()
{
}

void ARecipeCopierEquipment::BeginPlay()
{
	Super::BeginPlay();

	widgetFactoryInfo->SetWorldScale3D(FVector::ZeroVector);
	widgetSmartSplitterInfo->SetWorldScale3D(FVector::ZeroVector);
	widgetTrainInfo->SetWorldScale3D(FVector::ZeroVector);
	widgetSignInfo->SetWorldScale3D(FVector::ZeroVector);
	widgetLightsControlPanelInfo->SetWorldScale3D(FVector::ZeroVector);
	widgetValveInfo->SetWorldScale3D(FVector::ZeroVector);
}

void ARecipeCopierEquipment::HandleDefaultEquipmentActionEvent(EDefaultEquipmentAction action, EDefaultEquipmentActionEvent actionEvent)
{
	if (action == EDefaultEquipmentAction::PrimaryFire && actionEvent == EDefaultEquipmentActionEvent::Pressed)
	{
		PrimaryFirePressed();
	}
}

void ARecipeCopierEquipment::Equip(AFGCharacterPlayer* character)
{
	Super::Equip(character);

	HideOutline(character);
}

void ARecipeCopierEquipment::UnEquip()
{
	HideOutline(GetInstigatorCharacter());

	Super::UnEquip();
}

FString ARecipeCopierEquipment::GetAuthorityAndPlayer(const AActor* actor)
{
	return FString(TEXT("Has Authority = ")) +
		(actor->HasAuthority() ? TEXT("true") : TEXT("false")) +
		TEXT(" / Character = ") +
		(actor->GetInstigator() ? *actor->GetInstigator()->GetHumanReadableName() : TEXT("None"));
}

void ARecipeCopierEquipment::SetTargets
(
	AFGBuildableSplitterSmart* smartSplitter,
	AFGTrain* train,
	AFGBuildableWidgetSign* widgetSign,
	AFGBuildableLightsControlPanel* lightsControlPanel,
	class AFGBuildablePipelinePump* valve,
	AFGBuildableFactory* factory
)
{
	targetSmartSplitter = smartSplitter;
	targetTrain = train;
	targetWidgetSign = widgetSign;
	targetLightsControlPanel = lightsControlPanel;
	targetValve = valve;
	targetFactory = factory;
}

void ARecipeCopierEquipment::HandleAimSmartSplitter(class AFGCharacterPlayer* character, AFGBuildableSplitterSmart* smartSplitter)
{
	ShowOutline(character, smartSplitter);

	if (targetSmartSplitter != smartSplitter)
	{
		RC_LOG_Display(*GetPathNameSafe(smartSplitter), TEXT(": RecipeCopier: Set new target splitter smart"));

		SetTargets(
			smartSplitter,
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			nullptr
			);

		aimedSplitterRules.Empty();

		for (auto x = 0; x < smartSplitter->GetNumSortRules(); x++)
		{
			aimedSplitterRules.Add(smartSplitter->GetSortRuleAt(x));
		}

		if (!CompareSplitterRules(smartSplitter))
		{
			if (pointLight)
			{
				pointLight->SetIntensity(50);
				pointLight->SetLightColor(FColor(0x0, 0xff, 0x0, 0xff));
			}
		}
		else if (pointLight)
		{
			pointLight->SetIntensity(0);
		}

		SetWidgetSmartSplitterInfo(aimedSplitterRules, selectedSplitterRules);

		SetTextureWidget(currentWidgetInfo = widgetSmartSplitterInfo);
	}
}

void ARecipeCopierEquipment::HandleAimFactory(AFGPlayerController* playerController, AFGCharacterPlayer* character, AFGBuildableFactory* factory)
{
	auto manufacturer = Cast<AFGBuildableManufacturer>(factory);

	if (manufacturer || factory->GetCanChangePotential())
	{
		ShowOutline(character, factory);

		if (targetFactory != factory)
		{
			RC_LOG_Display_Condition(*GetPathNameSafe(factory), TEXT(": RecipeCopier: Set new target factory target"));

			SetTargets(
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				nullptr,
				factory
				);

			aimedRecipe = manufacturer ? manufacturer->GetCurrentRecipe() : nullptr;
			aimedOverclock = factory->GetPendingPotential();
			aimedProductionBoost = factory->GetPendingProductionBoost();

			// Check noop
			if ((selectedRecipe == nullptr && selectedOverclock <= 0 && selectedProductionBoost <= 1) || // No recipe and no overclocking
				((recipeCopyMode == ERecipeCopyMode::RecipeOnly ||
						((!factory->GetCanChangePotential() || aimedOverclock == selectedOverclock) &&
							(!factory->CanChangeProductionBoost() || aimedProductionBoost == selectedProductionBoost))) &&
					(recipeCopyMode == ERecipeCopyMode::OverclockOnly || !manufacturer || aimedRecipe == selectedRecipe)
					// Same overclock (or ignored) and same recipe (or ignored; or not a manufacturer)
				))
			{
				if (pointLight)
				{
					pointLight->SetIntensity(0);
				}
			}
			// Check can define recipe
			else if ((recipeCopyMode != ERecipeCopyMode::OverclockOnly && ARecipeCopierLogic::CanProduceRecipe(manufacturer, selectedRecipe) && aimedRecipe != selectedRecipe) ||
				(recipeCopyMode != ERecipeCopyMode::RecipeOnly &&
					((factory->GetCurrentMaxPotential() > 0 && aimedOverclock != selectedOverclock) ||
						(factory->GetCurrentProductionBoost() > 1 && aimedProductionBoost != selectedProductionBoost))))
			{
				if (pointLight)
				{
					pointLight->SetIntensity(50);
					pointLight->SetLightColor(FColor(0x00, 0xff, 0x00, 0xff));
				}
			}
			else
			{
				if (pointLight)
				{
					pointLight->SetIntensity(50);
					pointLight->SetLightColor(FColor(0xff, 0x00, 0x00, 0xff));
				}
			}

			SetWidgetFactoryInfo(
				aimedRecipe,
				selectedRecipe,
				aimedOverclock,
				selectedOverclock,
				aimedProductionBoost,
				selectedProductionBoost
				);

			SetTextureWidget(currentWidgetInfo = widgetFactoryInfo);
		}
	}
	else if (pointLight)
	{
		HideOutline(character);

		// Not a valid factory
		pointLight->SetIntensity(0);
	}
}

void ARecipeCopierEquipment::HandleAimSign(class AFGCharacterPlayer* character, AFGBuildableWidgetSign* widgetSign)
{
	ShowOutline(character, widgetSign);

	if (widgetSign != targetWidgetSign)
	{
		RC_LOG_Display_Condition(*GetPathNameSafe(widgetSign), TEXT(": RecipeCopier: Set new target sign"));

		SetTargets(
			nullptr,
			nullptr,
			widgetSign,
			nullptr,
			nullptr,
			nullptr
			);

		aimedIsDefined = true;

		FPrefabSignData signData;

		widgetSign->GetSignPrefabData(signData);

		// aimedPrefabLayout = signData.PrefabLayout;
		aimedSignTypeDesc = signData.SignTypeDesc;

		aimedForegroundColor = signData.ForegroundColor;
		aimedBackgroundColor = signData.BackgroundColor;
		aimedAuxiliaryColor = signData.AuxiliaryColor;
		aimedEmissive = signData.Emissive;
		aimedGlossiness = signData.Glossiness;

		// RC_LOG_Display_Condition(TEXT("    Prefab Layout: "), *GetPathNameSafe(aimedPrefabLayout ));
		// RC_LOG_Display_Condition(TEXT("    Prefab Layout Class: "), *GetPathNameSafe(aimedPrefabLayout ? aimedPrefabLayout->GetClass() : nullptr));
		// RC_LOG_Display_Condition(TEXT("    Sign Type Descriptor: "), *GetPathNameSafe(aimedSignTypeDesc));
		// RC_LOG_Display_Condition(TEXT("    Sign Type Descriptor Class: "), *GetPathNameSafe(aimedSignTypeDesc ? aimedSignTypeDesc->GetClass() : nullptr));

		aimedTexts = signData.TextElementData;
		aimedIconIDs = signData.IconElementData;

		// TArray<FString> OutKeys;
		// aimedTexts.GetKeys(OutKeys);
		// for (auto i : OutKeys)
		// {
		// 	RC_LOG_Display_Condition(TEXT("    Text element to data entry: "), *i, TEXT(" = "), *aimedTexts[i]);
		// }
		//
		// auto iconLibrary = UFGIconLibrary::Get();
		//
		// aimedIconIDs.GetKeys(OutKeys);
		// for (auto i : OutKeys)
		// {
		// 	RC_LOG_Display_Condition(TEXT("    Icon element to data entry: "), *i, TEXT(" = "), aimedIconIDs[i]);
		//
		// 	FIconData out_iconData;
		// 	UFGIconLibrary::GetIconDataForIconID(iconLibrary->GetClass(), aimedIconIDs[i], out_iconData);
		//
		// 	RC_LOG_Display_Condition(TEXT("        Icon data: "), *out_iconData.IconName.ToString());
		// 	RC_LOG_Display_Condition(TEXT("        Icon texture: "), *GetPathNameSafe(out_iconData.Texture.Get()));
		//
		// 	if (out_iconData.Texture)
		// 	{
		// 		RC_LOG_Display_Condition(TEXT("        Icon texture class: "), *GetPathNameSafe(out_iconData.Texture.Get()->GetClass()));
		// 	}
		// }

		// for (auto i : widgetSign->mPrefabTextElementSaveData)
		// {
		// 	RC_LOG_Display_Condition(TEXT("    Element name: "), *i.ElementName, TEXT(" = Element text: "), * i.Text);
		// }
		// for (auto i : widgetSign->mPrefabIconElementSaveData)
		// {
		// 	RC_LOG_Display_Condition(TEXT("    Element name: "), *i.ElementName, TEXT(" = Icon id: "), i.IconID);
		// }

		if (aimedForegroundColor != selectedForegroundColor ||
			aimedBackgroundColor != selectedBackgroundColor ||
			aimedAuxiliaryColor != selectedAuxiliaryColor ||
			aimedEmissive != selectedEmissive ||
			aimedGlossiness != selectedGlossiness ||
			!compareMaps(aimedTexts, selectedTexts) ||
			!compareMaps(aimedIconIDs, selectedIconIDs))
		{
			if (pointLight)
			{
				pointLight->SetIntensity(50);
				pointLight->SetLightColor(FColor(0x0, 0xff, 0x0, 0xff));
			}
		}
		else
		{
			if (pointLight)
			{
				pointLight->SetIntensity(0);
			}
		}

		SetWidgetSignInfo(
			aimedIsDefined,
			selectedIsDefined,
			aimedForegroundColor,
			selectedForegroundColor,
			aimedBackgroundColor,
			selectedBackgroundColor,
			aimedAuxiliaryColor,
			selectedAuxiliaryColor,
			aimedEmissive,
			selectedEmissive,
			aimedGlossiness,
			selectedGlossiness,
			aimedTexts,
			selectedTexts,
			aimedIconIDs,
			selectedIconIDs,
			// FText::FromString(GetNameSafe(aimedPrefabLayout.Get())),
			// FText::FromString(GetNameSafe(selectedPrefabLayout.Get())),
			signCopyMode
			);

		SetTextureWidget(currentWidgetInfo = widgetSignInfo);
	}
}

void ARecipeCopierEquipment::HandleAimTrain(class AFGCharacterPlayer* character, class AFGTrain* train)
{
	ShowOutline(character, train->GetMultipleUnitMaster());

	if (train != targetTrain)
	{
		RC_LOG_Display_Condition(*GetPathNameSafe(train), TEXT(": RecipeCopier: Set new target train"));

		SetTargets(
			nullptr,
			train,
			nullptr,
			nullptr,
			nullptr,
			nullptr
			);

		aimedTrainStops.Empty();

		if (train->HasTimeTable())
		{
			train->GetTimeTable()->GetStops(aimedTrainStops);

			if (!compareArrays(aimedTrainStops, selectedTrainStops))
			{
				if (pointLight)
				{
					pointLight->SetIntensity(50);
					pointLight->SetLightColor(FColor(0x0, 0xff, 0x0, 0xff));
				}
			}
			else
			{
				if (pointLight)
				{
					pointLight->SetIntensity(0);
				}
			}
		}
		else
		{
			if (pointLight)
			{
				pointLight->SetIntensity(0);
			}
		}

		SetWidgetTrainInfo(
			aimedTrainStops,
			selectedTrainStops
			);

		SetTextureWidget(currentWidgetInfo = widgetTrainInfo);
	}
}

void ARecipeCopierEquipment::HandleAimLightsControlPanel(class AFGCharacterPlayer* character, class AFGBuildableLightsControlPanel* lightsControlPanel)
{
	ShowOutline(character, lightsControlPanel);

	if (lightsControlPanel != targetLightsControlPanel)
	{
		RC_LOG_Display_Condition(*GetPathNameSafe(lightsControlPanel), TEXT(": RecipeCopier: Set new target lights control panel"));

		SetTargets(
			nullptr,
			nullptr,
			nullptr,
			lightsControlPanel,
			nullptr,
			nullptr
			);

		aimedLightSourceControlData = lightsControlPanel->GetLightControlData();
		aimedIsLightEnabled = lightsControlPanel->IsLightEnabled();

		if (aimedLightSourceControlData.Intensity != selectedLightSourceControlData.Intensity ||
			aimedLightSourceControlData.IsTimeOfDayAware != selectedLightSourceControlData.IsTimeOfDayAware ||
			aimedLightSourceControlData.ColorSlotIndex != selectedLightSourceControlData.ColorSlotIndex ||
			aimedIsLightEnabled != selectedIsLightEnabled)
		{
			if (pointLight)
			{
				pointLight->SetIntensity(50);
				pointLight->SetLightColor(FColor(0x0, 0xff, 0x0, 0xff));
			}
		}
		else
		{
			if (pointLight)
			{
				pointLight->SetIntensity(0);
			}
		}

		SetWidgetLightsControlPanelInfo(
			aimedLightSourceControlData,
			selectedLightSourceControlData,
			aimedIsLightEnabled,
			selectedIsLightEnabled
			);

		SetTextureWidget(currentWidgetInfo = widgetLightsControlPanelInfo);
	}
}

void ARecipeCopierEquipment::HandleAimValve(class AFGCharacterPlayer* character, class AFGBuildablePipelinePump* valve)
{
	ShowOutline(character, valve);

	if (valve != targetValve)
	{
		RC_LOG_Display_Condition(*GetPathNameSafe(valve), TEXT(": RecipeCopier: Set new target valve"));

		SetTargets(
			nullptr,
			nullptr,
			nullptr,
			nullptr,
			valve,
			nullptr
			);

		aimedUserFlowLimit = valve->GetUserFlowLimit();

		if (aimedUserFlowLimit != selectedUserFlowLimit)
		{
			if (pointLight)
			{
				pointLight->SetIntensity(50);
				pointLight->SetLightColor(FColor(0x0, 0xff, 0x0, 0xff));
			}
		}
		else
		{
			if (pointLight)
			{
				pointLight->SetIntensity(0);
			}
		}

		SetWidgetValveInfo(
			aimedUserFlowLimit,
			selectedUserFlowLimit
			);

		SetTextureWidget(currentWidgetInfo = widgetValveInfo);
	}
}

void ARecipeCopierEquipment::HandleHitActor(AActor* hitActor, bool& wasHit)
{
	if (auto playerController = Cast<AFGPlayerController>(GetInstigatorController()))
	{
		auto character = GetInstigatorCharacter();

		bool clearTarget = false;

		if (hitActor && playerController->WasInputKeyJustPressed(EKeys::NumPadZero))
		{
			UMarcioCommonLibsUtils::DumpUnknownClass(hitActor);
			// ARecipeCopierLogic::DumpUnknownClass(hitActor);
		}

		// if (playerController->WasInputKeyJustPressed(toggleKey))
		// {
		// 	CycleCopyMode();
		// }

		if (auto smartSplitter = Cast<AFGBuildableSplitterSmart>(hitActor))
		{
			HandleAimSmartSplitter(character, smartSplitter);
		}
		else if (auto locomotive = Cast<AFGLocomotive>(hitActor))
		{
			HandleAimTrain(character, locomotive->GetTrain());
		}
		else if (auto widgetSign = Cast<AFGBuildableWidgetSign>(hitActor))
		{
			HandleAimSign(character, widgetSign);
		}
		else if (auto lightsControlPanel = Cast<AFGBuildableLightsControlPanel>(hitActor))
		{
			HandleAimLightsControlPanel(character, lightsControlPanel);
		}
		else if (auto valve = Cast<AFGBuildablePipelinePump>(hitActor))
		{
			if (hitActor->GetClass()->IsChildOf(ARecipeCopierLogic::valveClass))
			{
				HandleAimValve(character, valve);
			}
			else
			{
				clearTarget = true;
			}
		}
		else if (auto factory = Cast<AFGBuildableFactory>(hitActor))
		{
			HandleAimFactory(playerController, character, factory);
		}
		else
		{
			clearTarget = true;
		}

		wasHit = !clearTarget;

		if (clearTarget)
		{
			HideOutline(character);

			if (pointLight)
			{
				pointLight->SetIntensity(0);
			}

			ClearTargets();
		}
	}
}

void ARecipeCopierEquipment::PrimaryFirePressed()
{
	if (auto playerController = Cast<AFGPlayerController>(GetInstigatorController()))
	{
		if (playerController->IsInputKeyDown(EKeys::LeftControl))
		{
			CopyTarget();
		}
		else
		{
			ApplyTarget();
		}
	}
}

void ARecipeCopierEquipment::CopyTarget()
{
	RC_LOG_Display_Condition(
		ELogVerbosity::Log,
		*getTagName(),
		TEXT("CopyTarget = "),
		*GetPathName(),
		TEXT(" / "),
		*GetAuthorityAndPlayer(this)
		);

	if (targetFactory)
	{
		CopyFactory();
	}
	else if (targetSmartSplitter)
	{
		CopySmartSplitter();
	}
	else if (targetWidgetSign)
	{
		CopyWidgetSign();
	}
	else if (targetTrain)
	{
		CopyTrain();
	}
	else if (targetLightsControlPanel)
	{
		CopyLightsControlPanel();
	}
	else if (targetValve)
	{
		CopyValve();
	}
	else
	{
		ClearTargets();
	}
}

void ARecipeCopierEquipment::CopyFactory()
{
	RC_LOG_Display_Condition(*GetPathNameSafe(targetSmartSplitter), TEXT(": RecipeCopier: Set new target factory"));

	selectedRecipe = aimedRecipe;
	selectedOverclock = aimedOverclock;
	selectedProductionBoost = aimedProductionBoost;

	SetWidgetFactoryInfo(
		aimedRecipe,
		selectedRecipe,
		aimedOverclock,
		selectedOverclock,
		aimedProductionBoost,
		selectedProductionBoost
		);
}

void ARecipeCopierEquipment::CopySmartSplitter()
{
	RC_LOG_Display_Condition(*GetPathNameSafe(targetSmartSplitter), TEXT(": RecipeCopier: Set new target splitter smart"));

	selectedSplitterRules = aimedSplitterRules;

	SetWidgetSmartSplitterInfo(
		aimedSplitterRules,
		selectedSplitterRules
		);
}

void ARecipeCopierEquipment::CopyWidgetSign()
{
	RC_LOG_Display_Condition(*GetPathNameSafe(targetSmartSplitter), TEXT(": RecipeCopier: Set new target widget sign"));

	selectedIsDefined = aimedIsDefined;
	selectedForegroundColor = aimedForegroundColor;
	selectedBackgroundColor = aimedBackgroundColor;
	selectedAuxiliaryColor = aimedAuxiliaryColor;
	selectedEmissive = aimedEmissive;
	selectedGlossiness = aimedGlossiness;
	selectedTexts = aimedTexts;
	selectedIconIDs = aimedIconIDs;
	// selectedPrefabLayout = aimedPrefabLayout;
	selectedSignTypeDesc = aimedSignTypeDesc;

	SetWidgetSignInfo(
		aimedIsDefined,
		selectedIsDefined,
		aimedForegroundColor,
		selectedForegroundColor,
		aimedBackgroundColor,
		selectedBackgroundColor,
		aimedAuxiliaryColor,
		selectedAuxiliaryColor,
		aimedEmissive,
		selectedEmissive,
		aimedGlossiness,
		selectedGlossiness,
		aimedTexts,
		selectedTexts,
		aimedIconIDs,
		selectedIconIDs,
		// FText::FromString(GetNameSafe(aimedPrefabLayout.Get())),
		// FText::FromString(GetNameSafe(selectedPrefabLayout.Get())),
		signCopyMode
		);
}

void ARecipeCopierEquipment::CopyTrain()
{
	RC_LOG_Display_Condition(*GetPathNameSafe(targetSmartSplitter), TEXT(": RecipeCopier: Set new target train"));

	selectedTrainStops = aimedTrainStops;

	SetWidgetTrainInfo(
		aimedTrainStops,
		selectedTrainStops
		);
}

void ARecipeCopierEquipment::CopyLightsControlPanel()
{
	RC_LOG_Display_Condition(*GetPathNameSafe(targetSmartSplitter), TEXT(": RecipeCopier: Set new target lights control panel"));

	selectedLightSourceControlData = aimedLightSourceControlData;
	selectedIsLightEnabled = aimedIsLightEnabled;

	SetWidgetLightsControlPanelInfo(
		aimedLightSourceControlData,
		selectedLightSourceControlData,
		aimedIsLightEnabled,
		selectedIsLightEnabled
		);
}

void ARecipeCopierEquipment::CopyValve()
{
	RC_LOG_Display_Condition(*GetPathNameSafe(targetSmartSplitter), TEXT(": RecipeCopier: Set new target valve"));

	selectedUserFlowLimit = aimedUserFlowLimit;

	SetWidgetValveInfo(
		selectedUserFlowLimit,
		aimedUserFlowLimit
		);
}

void ARecipeCopierEquipment::ClearTargets_Implementation()
{
	SetTargets(
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr,
		nullptr
		);

	SetWidgetFactoryInfo(
		aimedRecipe = nullptr,
		selectedRecipe,
		aimedOverclock = 0,
		selectedOverclock,
		aimedProductionBoost = 0,
		selectedProductionBoost
		);

	SetWidgetSmartSplitterInfo(
		aimedSplitterRules = TArray<FSplitterSortRule>(),
		selectedSplitterRules
		);

	// aimedPrefabLayout=nullptr;

	SetWidgetSignInfo(
		aimedIsDefined = false,
		selectedIsDefined,
		aimedForegroundColor = FLinearColor(),
		selectedForegroundColor,
		aimedBackgroundColor = FLinearColor(),
		selectedBackgroundColor,
		aimedAuxiliaryColor = FLinearColor(),
		selectedAuxiliaryColor,
		aimedEmissive = 0,
		selectedEmissive,
		aimedGlossiness = 0,
		selectedGlossiness,
		aimedTexts = TMap<FString, FString>(),
		selectedTexts,
		aimedIconIDs = TMap<FString, int32>(),
		selectedIconIDs,
		// FText::FromString(GetNameSafe(aimedPrefabLayout.Get())),
		// FText::FromString(GetNameSafe(selectedPrefabLayout.Get())),
		signCopyMode
		);

	aimedSignTypeDesc = nullptr;

	SetWidgetTrainInfo(
		aimedTrainStops = TArray<FTimeTableStop>(),
		selectedTrainStops
		);

	SetWidgetLightsControlPanelInfo(
		aimedLightSourceControlData = FLightSourceControlData(),
		selectedLightSourceControlData,
		aimedIsLightEnabled = false,
		selectedIsLightEnabled
		);

	SetWidgetValveInfo(
		aimedUserFlowLimit = 0,
		selectedUserFlowLimit
		);

	if (pointLight)
	{
		pointLight->SetIntensity(0);
	}
}

void ARecipeCopierEquipment::ApplyTarget()
{
	RC_LOG_Display_Condition(
		ELogVerbosity::Log,
		*getTagName(),
		TEXT("ApplyTarget = "),
		*GetPathName(),
		TEXT(" / "),
		*GetAuthorityAndPlayer(this)
		);

	if (targetFactory)
	{
		ARecipeCopierLogic::ApplyFactoryInfo(
			targetFactory,
			selectedRecipe,
			selectedOverclock,
			selectedProductionBoost,
			recipeCopyMode,
			GetInstigatorCharacter(),
			this
			);
	}
	else if (targetSmartSplitter)
	{
		ARecipeCopierLogic::ApplySmartSplitterInfo(
			targetSmartSplitter,
			selectedSplitterRules,
			GetInstigatorCharacter(),
			this
			);
	}
	else if (targetWidgetSign)
	{
		ARecipeCopierLogic::ApplyWidgetSignInfo(
			targetWidgetSign,
			selectedForegroundColor,
			selectedBackgroundColor,
			selectedAuxiliaryColor,
			selectedEmissive,
			selectedGlossiness,
			selectedTexts,
			selectedIconIDs,
			// selectedPrefabLayout,
			selectedSignTypeDesc,
			signCopyMode,
			GetInstigatorCharacter(),
			this
			);
	}
	else if (targetTrain)
	{
		ARecipeCopierLogic::ApplyTrainInfo(
			targetTrain,
			selectedTrainStops,
			GetInstigatorCharacter(),
			this
			);
	}
	else if (targetLightsControlPanel)
	{
		ARecipeCopierLogic::ApplyLightsControlPanel(
			targetLightsControlPanel,
			selectedLightSourceControlData,
			selectedIsLightEnabled,
			GetInstigatorCharacter(),
			this
			);
	}
	else if (targetValve)
	{
		ARecipeCopierLogic::ApplyValve(
			targetValve,
			selectedUserFlowLimit,
			GetInstigatorCharacter(),
			this
			);
	}
}

void ARecipeCopierEquipment::CycleCopyMode()
{
	auto playerController = Cast<AFGPlayerController>(GetInstigatorController());
	if (!playerController)
	{
		return;
	}

	auto isLeftControlDown = playerController->IsInputKeyDown(EKeys::LeftControl);
	auto isLeftShiftDown = playerController->IsInputKeyDown(EKeys::LeftShift);
	auto isLeftAltDown = playerController->IsInputKeyDown(EKeys::LeftAlt);

	if (currentWidgetInfo == widgetFactoryInfo)
	{
		if (isLeftControlDown && isLeftShiftDown && isLeftAltDown)
		{
			// Reset to default
			recipeCopyMode = ERecipeCopyMode::RecipeAndOverclock;
		}
		else
		{
			switch (recipeCopyMode)
			{
			case ERecipeCopyMode::RecipeOnly:
				recipeCopyMode = ERecipeCopyMode::OverclockOnly;
				break;
			case ERecipeCopyMode::OverclockOnly:
				recipeCopyMode = ERecipeCopyMode::RecipeAndOverclock;
				break;
			case ERecipeCopyMode::RecipeAndOverclock:
				recipeCopyMode = ERecipeCopyMode::RecipeOnly;
				break;
			}
		}

		SetWidgetFactoryInfo(
			aimedRecipe,
			selectedRecipe,
			aimedOverclock,
			selectedOverclock,
			aimedProductionBoost,
			selectedProductionBoost
			);

		PlayObjectScannerCycleRightAnim();
	}
	else if (currentWidgetInfo == widgetSignInfo)
	{
		auto signCopyModeBackup = signCopyMode;

		if (isLeftControlDown && isLeftShiftDown && isLeftAltDown)
		{
			// Reset to default
			signCopyMode = TO_ESignCopyModeType(ESignCopyModeType::SCMT_All);
		}
		else if (isLeftControlDown && !isLeftShiftDown && !isLeftAltDown)
		{
			signCopyMode = Toggle_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Colors);
		}
		else if (!isLeftControlDown && !isLeftShiftDown && isLeftAltDown)
		{
			signCopyMode = Toggle_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Icons);
		}
		else if (isLeftControlDown && isLeftShiftDown && !isLeftAltDown)
		{
			signCopyMode = Toggle_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_EmissiveAndGlossiness);
		}
		else if (!isLeftControlDown && isLeftShiftDown && !isLeftAltDown)
		{
			signCopyMode = Toggle_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Texts);
		}
		else if (!isLeftControlDown && !isLeftShiftDown && !isLeftAltDown)
		{
			signCopyMode = Toggle_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Layout);
		}

		if (!Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Colors) &&
			!Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Icons) &&
			!Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_EmissiveAndGlossiness) &&
			!Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Texts) &&
			!Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Layout))
		{
			signCopyMode = signCopyModeBackup;
		}

		SetWidgetSignInfo(
			aimedIsDefined,
			selectedIsDefined,
			aimedForegroundColor,
			selectedForegroundColor,
			aimedBackgroundColor,
			selectedBackgroundColor,
			aimedAuxiliaryColor,
			selectedAuxiliaryColor,
			aimedEmissive,
			selectedEmissive,
			aimedGlossiness,
			selectedGlossiness,
			aimedTexts,
			selectedTexts,
			aimedIconIDs,
			selectedIconIDs,
			// FText::FromString(GetNameSafe(aimedPrefabLayout.Get())),
			// FText::FromString(GetNameSafe(selectedPrefabLayout.Get())),
			signCopyMode
			);

		PlayObjectScannerCycleRightAnim();
	}
}

bool ARecipeCopierEquipment::CompareSplitterRules(class AFGBuildableSplitterSmart* smartSplitter)
{
	if (smartSplitter->GetNumSortRules() != selectedSplitterRules.Num())
	{
		return false;
	}

	for (auto x = 0; x < selectedSplitterRules.Num(); x++)
	{
		if (smartSplitter->GetSortRuleAt(x).ItemClass != selectedSplitterRules[x].ItemClass ||
			smartSplitter->GetSortRuleAt(x).OutputIndex != selectedSplitterRules[x].OutputIndex)
		{
			return false;
		}
	}

	return true;
}

void ARecipeCopierEquipment::ShowOutline(AFGCharacterPlayer* character, AActor* actor)
{
	HideOutline(character);

	if (character && actor)
	{
		character->GetOutline()->ShowOutline(actor, EOutlineColor::OC_USABLE);
		outlinedActor = actor;
	}
}

void ARecipeCopierEquipment::HideOutline(AFGCharacterPlayer* character)
{
	if (character && outlinedActor)
	{
		character->GetOutline()->HideOutline(outlinedActor);
		outlinedActor = nullptr;
	}
}


#ifndef OPTIMIZE
#pragma optimize("", on)
#endif
