#include "RecipeCopierEquipment.h"
#include "RecipeCopierModule.h"
#include "RecipeCopierRCO.h"
#include "Logic/RecipeCopierLogic.h"
#include "Util/Optimize.h"
#include "Util/Logging.h"

#include <map>

#include "FGIconLibrary.h"
#include "FGPlayerController.h"
#include "FGTrain.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "Components/WidgetComponent.h"

#ifndef OPTIMIZE
#pragma optimize( "", off )
#endif

template <typename K, typename V>
bool compareMaps(const TMap<K, V>& a, const TMap<K, V>& b)
{
	if (a.Num() != b.Num())
	{
		return false;
	}

	TArray<K> keys;
	a.GetKeys(keys);

	for (auto key : keys)
	{
		if (!b.Contains(key) || a[key] != b[key])
		{
			return false;
		}
	}

	return true;
}

template <typename K, typename V>
TArray<V> mapValues(const TMap<K, V>& m)
{
	TArray<K> keys;
	m.GetKeys(keys);

	TArray<V> values;

	for (auto key : keys)
	{
		values.Add(m[key]);
	}

	return values;
}

/*template<>
bool compareArrays<FString>(const TArray<FString>& a, const TArray<FString>& b)
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
}*/

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
}

FString ARecipeCopierEquipment::GetAuthorityAndPlayer(const AActor* actor)
{
	return FString(TEXT("Has Authority = ")) +
		(actor->HasAuthority() ? TEXT("true") : TEXT("false")) +
		TEXT(" / Character = ") +
		(actor->GetInstigator() ? *actor->GetInstigator()->GetHumanReadableName() : TEXT("None"));
}

bool ARecipeCopierEquipment::IsLogLevelDisplay()
{
	return ARecipeCopierLogic::GetLogLevel() > 4;
}

void ARecipeCopierEquipment::SetTargets
(
	AFGBuildableSplitterSmart* smartSplitter,
	AFGTrain* train,
	AFGBuildableWidgetSign* widgetSign,
	AFGBuildableFactory* factory
)
{
	targetFactory = factory;
	targetSmartSplitter = smartSplitter;
	targetWidgetSign = widgetSign;
	targetTrain = train;
}

void ARecipeCopierEquipment::HandleAimSmartSplitter(class AFGCharacterPlayer* character, AFGBuildableSplitterSmart* smartSplitter)
{
	character->GetOutline()->ShowOutline(smartSplitter, EOutlineColor::OC_USABLE);

	if (targetSmartSplitter != smartSplitter)
	{
		RC_LOG_Display(*GetPathNameSafe(smartSplitter), TEXT(": RecipeCopier: Set new target splitter smart"));

		SetTargets(
			smartSplitter,
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

		SetTextureWidget(widgetSmartSplitterInfo);
	}
}

void ARecipeCopierEquipment::HandleAimFactory(AFGPlayerController* playerController, AFGCharacterPlayer* character, AFGBuildableFactory* factory)
{
	auto manufacturer = Cast<AFGBuildableManufacturer>(factory);

	if (manufacturer || factory->GetCanChangePotential())
	{
		character->GetOutline()->ShowOutline(factory, EOutlineColor::OC_USABLE);

		if (targetFactory != factory)
		{
			RC_LOG_Display_Condition(*GetPathNameSafe(factory), TEXT(": RecipeCopier: Set new target factory target"));

			SetTargets(
				nullptr,
				nullptr,
				nullptr,
				factory
				);

			aimedRecipe = manufacturer ? manufacturer->GetCurrentRecipe() : nullptr;
			aimedOverclock = factory->GetPendingPotential();

			// Check noop
			if (selectedRecipe == nullptr && selectedOverclock <= 0 || // No recipe and no overclocking
				(copyMode == ERecipeCopyMode::RecipeOnly || !factory->GetCanChangePotential() || aimedOverclock == selectedOverclock) &&
				(copyMode == ERecipeCopyMode::OverclockOnly || !manufacturer || aimedRecipe == selectedRecipe)
				// Same overclock (or ignored) and same recipe (or ignored; or not a manufacturer)
				)
			{
				if (pointLight)
				{
					pointLight->SetIntensity(0);
				}
			}
			// Check can define recipe
			else if (copyMode != ERecipeCopyMode::OverclockOnly && ARecipeCopierLogic::CanProduceRecipe(manufacturer, selectedRecipe) && aimedRecipe != selectedRecipe ||
				copyMode != ERecipeCopyMode::RecipeOnly && factory->GetMaxPossiblePotential() > 0 && aimedOverclock != selectedOverclock)
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
				selectedOverclock
				);

			SetTextureWidget(widgetFactoryInfo);
		}
	}
	else if (pointLight)
	{
		character->GetOutline()->HideOutline();

		// Not a valid factory
		pointLight->SetIntensity(0);
	}
}

void ARecipeCopierEquipment::HandleAimSign(class AFGCharacterPlayer* character, AFGBuildableWidgetSign* widgetSign)
{
	character->GetOutline()->ShowOutline(widgetSign, EOutlineColor::OC_USABLE);

	if (widgetSign != targetWidgetSign)
	{
		RC_LOG_Display_Condition(*GetPathNameSafe(widgetSign), TEXT(": RecipeCopier: Set new target sign"));

		SetTargets(
			nullptr,
			nullptr,
			widgetSign,
			nullptr
			);

		aimedIsDefined = true;

		FPrefabSignData signData;

		widgetSign->GetSignPrefabData(signData);
		
		aimedForegroundColor = signData.ForegroundColor;
		aimedBackgroundColor = signData.BackgroundColor;
		aimedAuxiliaryColor = signData.AuxiliaryColor;
		aimedEmissive = signData.Emissive;
		aimedGlossiness = signData.Glossiness;
		aimedLayout = signData.PrefabLayout;

		// RC_LOG_Display_Condition(TEXT("    Active Prefab Layout: "), *GetNameSafe(signData.mActivePrefabLayout));

		aimedTexts = signData.TextElementData;
		aimedIconIDs = signData.IconElementData;

		TArray<FString> OutKeys;
		aimedTexts.GetKeys(OutKeys);
		for (auto i : OutKeys)
		{
			RC_LOG_Display_Condition(TEXT("    Text element to data entry: "), *i, TEXT(" = "), *aimedTexts[i]);
		}

		auto iconLibrary = UFGIconLibrary::Get();

		aimedIconIDs.GetKeys(OutKeys);
		for (auto i : OutKeys)
		{
			RC_LOG_Display_Condition(TEXT("    Icon element to data entry: "), *i, TEXT(" = "), aimedIconIDs[i]);

			FIconData out_iconData;
			UFGIconLibrary::GetIconDataForIconID(iconLibrary->GetClass(), aimedIconIDs[i], out_iconData);

			RC_LOG_Display_Condition(TEXT("        Icon data: "), *out_iconData.IconName.ToString());
			RC_LOG_Display_Condition(TEXT("        Icon texture: "), *GetPathNameSafe(out_iconData.Texture.Get()));

			if (out_iconData.Texture)
			{
				RC_LOG_Display_Condition(TEXT("        Icon texture class: "), *GetPathNameSafe(out_iconData.Texture.Get()->GetClass()));
			}
		}

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
			mapValues(aimedTexts),
			mapValues(selectedTexts),
			mapValues(aimedIconIDs),
			mapValues(selectedIconIDs)
			);

		SetTextureWidget(widgetSignInfo);
	}
}

void ARecipeCopierEquipment::HandleAimTrain(class AFGCharacterPlayer* character, class AFGTrain* train)
{
}

void ARecipeCopierEquipment::HandleHitActor(AActor* hitActor)
{
	if (auto playerController = Cast<AFGPlayerController>(GetInstigatorController()))
	{
		auto character = GetInstigatorCharacter();

		bool clearTarget = false;

		if (hitActor && playerController->WasInputKeyJustPressed(EKeys::NumPadZero))
		{
			ARecipeCopierLogic::DumpUnknownClass(hitActor);
		}

		if (playerController->WasInputKeyJustPressed(toggleKey))
		{
			CycleCopyMode();
		}

		if (auto smartSplitter = Cast<AFGBuildableSplitterSmart>(hitActor))
		{
			HandleAimSmartSplitter(character, smartSplitter);
		}
		else if (auto train = Cast<AFGTrain>(hitActor))
		{
			character->GetOutline()->ShowOutline(train, EOutlineColor::OC_USABLE);
		}
		else if (auto widgetSign = Cast<AFGBuildableWidgetSign>(hitActor))
		{
			HandleAimSign(character, widgetSign);
		}
		else if (auto factory = Cast<AFGBuildableFactory>(hitActor))
		{
			HandleAimFactory(playerController, character, factory);
		}
		else
		{
			clearTarget = true;
		}

		if (clearTarget)
		{
			character->GetOutline()->HideOutline();

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

	SetWidgetFactoryInfo(
		aimedRecipe,
		selectedRecipe,
		aimedOverclock,
		selectedOverclock
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
		mapValues(aimedTexts),
		mapValues(selectedTexts),
		mapValues(aimedIconIDs),
		mapValues(selectedIconIDs)
		);
}

void ARecipeCopierEquipment::CopyTrain()
{
}

void ARecipeCopierEquipment::ClearTargets_Implementation()
{
	SetTargets(
		nullptr,
		nullptr,
		nullptr,
		nullptr
		);

	SetWidgetFactoryInfo(
		aimedRecipe = nullptr,
		selectedRecipe,
		aimedOverclock = 0,
		selectedOverclock
		);

	SetWidgetSmartSplitterInfo(
		aimedSplitterRules = TArray<FSplitterSortRule>(),
		selectedSplitterRules
		);

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
		mapValues(aimedTexts = TMap<FString, FString>()),
		mapValues(selectedTexts),
		mapValues(aimedIconIDs = TMap<FString, int32>()),
		mapValues(selectedIconIDs)
		);

	SetWidgetTrainInfo(
		);
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
			copyMode,
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
			GetInstigatorCharacter(),
			this
			);
	}
	else if (targetTrain)
	{
		ARecipeCopierLogic::ApplyTrainInfo(
			targetTrain,
			GetInstigatorCharacter(),
			this
			);
	}
}

void ARecipeCopierEquipment::CycleCopyMode()
{
	switch (copyMode)
	{
	case ERecipeCopyMode::RecipeOnly:
		copyMode = ERecipeCopyMode::OverclockOnly;
		break;
	case ERecipeCopyMode::OverclockOnly:
		copyMode = ERecipeCopyMode::RecipeAndOverclock;
		break;
	case ERecipeCopyMode::RecipeAndOverclock:
		copyMode = ERecipeCopyMode::RecipeOnly;
		break;
	}

	SetWidgetFactoryInfo(
		aimedRecipe,
		selectedRecipe,
		aimedOverclock,
		selectedOverclock
		);

	PlayObjectScannerCycleRightAnim();
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

#ifndef OPTIMIZE
#pragma optimize( "", on)
#endif
