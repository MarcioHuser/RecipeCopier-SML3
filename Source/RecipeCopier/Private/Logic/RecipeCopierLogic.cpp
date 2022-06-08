// ReSharper disable CppUE4CodingStandardNamingViolationWarning
// ReSharper disable CommentTypo
// ReSharper disable IdentifierTypo

#include "Logic/RecipeCopierLogic.h"

#include "FGCharacterPlayer.h"
#include "FGItemPickup_Spawnable.h"
#include "FGTrain.h"
#include "RecipeCopierEquipment.h"
#include "RecipeCopierRCO.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "Util/Optimize.h"
#include "Util/Logging.h"

#include "Resources/FGItemDescriptor.h"

#ifndef OPTIMIZE
#pragma optimize( "", off )
#endif

ARecipeCopierLogic* ARecipeCopierLogic::singleton = nullptr;
FRecipeCopier_ConfigStruct ARecipeCopierLogic::configuration;
TSubclassOf<UFGItemDescriptor> ARecipeCopierLogic::shardItemDescriptor;
TSubclassOf<AFGBuildableSplitterSmart> ARecipeCopierLogic::programmableSplitterClass;

inline FString getEnumItemName(TCHAR* name, int value)
{
	FString valueStr;

	auto MyEnum = FindObject<UEnum>(ANY_PACKAGE, name);
	if (MyEnum)
	{
		MyEnum->AddToRoot();

		valueStr = MyEnum->GetDisplayNameTextByValue(value).ToString();
	}
	else
	{
		valueStr = TEXT("(Unknown)");
	}

	return FString::Printf(TEXT("%s (%d)"), *valueStr, value);
}

void ARecipeCopierLogic::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Terminate();
}

void ARecipeCopierLogic::Initialize
(
	TSubclassOf<UFGItemDescriptor> in_shardItemDescriptor,
	TSubclassOf<AFGBuildableSplitterSmart> in_programmableSplitterClass
)
{
	shardItemDescriptor = in_shardItemDescriptor;
	programmableSplitterClass = in_programmableSplitterClass;
	singleton = this;
}

void ARecipeCopierLogic::Terminate()
{
	FScopeLock ScopeLock(&eclCritical);

	singleton = nullptr;
}

void ARecipeCopierLogic::SetConfiguration(const FRecipeCopier_ConfigStruct& in_configuration)
{
	configuration = in_configuration;

	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	RC_LOG_Display(TEXT("StartupModule"));

	RC_LOG_Display(TEXT("logLevel = "), configuration.logLevel);

	RC_LOG_Display(TEXT("==="));
}

int ARecipeCopierLogic::GetLogLevel()
{
	return configuration.logLevel;
}

void ARecipeCopierLogic::DumpUnknownClass(UObject* obj)
{
	if (IS_RC_LOG_LEVEL(ELogVerbosity::Log))
	{
		RC_LOG_Display(TEXT("Unknown Class "), *obj->GetClass()->GetPathName());

		for (auto cls = obj->GetClass()->GetSuperClass(); cls && cls != AActor::StaticClass(); cls = cls->GetSuperClass())
		{
			RC_LOG_Display(TEXT("    - Super: "), *cls->GetPathName());
		}

		for (TFieldIterator<FProperty> property(obj->GetClass()); property; ++property)
		{
			RC_LOG_Display(
				TEXT("    - "),
				*property->GetName(),
				TEXT(" ("),
				*property->GetCPPType(),
				TEXT(" / "),
				*property->GetClass()->GetName(),
				TEXT(")")
				);

			auto floatProperty = CastField<FFloatProperty>(*property);
			if (floatProperty)
			{
				RC_LOG_Display(TEXT("        = "), floatProperty->GetPropertyValue_InContainer(obj));
			}

			auto intProperty = CastField<FIntProperty>(*property);
			if (intProperty)
			{
				RC_LOG_Display(TEXT("        = "), intProperty->GetPropertyValue_InContainer(obj));
			}

			auto boolProperty = CastField<FBoolProperty>(*property);
			if (boolProperty)
			{
				RC_LOG_Display(TEXT("        = "), boolProperty->GetPropertyValue_InContainer(obj) ? TEXT("true") : TEXT("false"));
			}

			auto structProperty = CastField<FStructProperty>(*property);
			if (structProperty && property->GetCPPType() == TEXT("FFactoryTickFunction"))
			{
				auto factoryTick = structProperty->ContainerPtrToValuePtr<FFactoryTickFunction>(obj);
				if (factoryTick)
				{
					RC_LOG_Display(TEXT("        - Tick Interval = "), factoryTick->TickInterval);
				}
			}

			auto strProperty = CastField<FStrProperty>(*property);
			if (strProperty)
			{
				RC_LOG_Display(TEXT("        = "), *strProperty->GetPropertyValue_InContainer(obj));
			}

			auto textProperty = CastField<FTextProperty>(*property);
			if (textProperty)
			{
				RC_LOG_Display(TEXT("        = "), *textProperty->GetPropertyValue_InContainer(obj).ToString());
			}

			auto classProperty = CastField<FClassProperty>(*property);
			if (classProperty)
			{
				RC_LOG_Display(TEXT("        = "), *GetNameSafe(classProperty->GetPropertyValue_InContainer(obj)));
			}

			auto widgetComponentProperty = CastField<FObjectProperty>(*property);
			if (widgetComponentProperty && property->GetCPPType() == TEXT("UWidgetComponent*"))
			{
				auto widgetComponent = widgetComponentProperty->ContainerPtrToValuePtr<UWidgetComponent>(obj);
				if (widgetComponent)
				{
					RC_LOG_Display(TEXT("            - "), *GetPathNameSafe(widgetComponent->GetClass()));
				}
			}
		}
	}
}

void ARecipeCopierLogic::ApplyFactoryInfo
(
	class AFGBuildableFactory* factory,
	const TSubclassOf<UFGRecipe>& recipe,
	float overclock,
	ERecipeCopyMode copyMode,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (factory->HasAuthority())
	{
		ApplyFactoryInfo_Server(
			factory,
			recipe,
			overclock,
			copyMode,
			player,
			copier
			);
	}
	else
	{
		auto rco = URecipeCopierRCO::getRCO(factory);
		if (rco)
		{
			rco->ApplyFactoryInfo(
				factory,
				recipe,
				overclock,
				copyMode,
				player,
				copier
				);
		}
	}
}

void ARecipeCopierLogic::ApplyFactoryInfo_Server
(
	class AFGBuildableFactory* factory,
	const TSubclassOf<UFGRecipe>& recipe,
	float overclock,
	ERecipeCopyMode copyMode,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (!factory || !factory->HasAuthority())
	{
		return;
	}

	auto manufacturer = Cast<AFGBuildableManufacturer>(factory);
	if (copyMode != ERecipeCopyMode::OverclockOnly &&
		recipe &&
		manufacturer &&
		manufacturer->GetCurrentRecipe() != recipe &&
		CanProduceRecipe(manufacturer, recipe))
	{
		manufacturer->MoveOrDropInputInventory(player);
		manufacturer->MoveOrDropOutputInventory(player);
		manufacturer->SetRecipe(recipe);
	}

	if (copyMode != ERecipeCopyMode::RecipeOnly &&
		factory->GetCanChangePotential() &&
		factory->GetPendingPotential() != overclock &&
		overclock > 0)
	{
		auto playerInventory = player->GetInventory();
		auto potentialIventory = factory->GetPotentialInventory();

		RC_LOG_Display_Condition(TEXT("Applying overclock"));

		// Adjust maximum/maximum overclock
		overclock = FMath::Max(FMath::Min(overclock, factory->GetMaxPossiblePotential()), factory->GetMinPotential());

		// how many shard are necessary/extra
		auto necessaryShards = FMath::Max(0, FMath::Min(3, FMath::CeilToInt((overclock - 1) / 0.5))) - potentialIventory->GetNumItems(shardItemDescriptor);
		necessaryShards = FMath::Min(necessaryShards, playerInventory->GetNumItems(shardItemDescriptor));

		if (necessaryShards > 0)
		{
			RC_LOG_Display_Condition(TEXT("Adding "), necessaryShards, TEXT(" shards to "), *GetPathNameSafe(factory));

			// Add shards
			MoveItems_Server(
				playerInventory,
				potentialIventory,
				shardItemDescriptor,
				necessaryShards,
				false,
				nullptr
				);
		}
		else if (necessaryShards < 0)
		{
			RC_LOG_Display_Condition(TEXT("Removing "), -necessaryShards, TEXT(" shards from "), *GetPathNameSafe(factory));

			// Remove shards
			MoveItems_Server(
				potentialIventory,
				playerInventory,
				shardItemDescriptor,
				-necessaryShards,
				true,
				player
				);
		}

		factory->SetPendingPotential(FMath::Min(overclock, factory->GetCurrentMaxPotential()));
	}

	copier->ClearTargets();
}

void ARecipeCopierLogic::ApplySmartSplitterInfo
(
	AFGBuildableSplitterSmart* smartSplitter,
	const TArray<FSplitterSortRule> splitterRules,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (smartSplitter->HasAuthority())
	{
		ApplySmartSplitterInfo_Server(
			smartSplitter,
			splitterRules,
			player,
			copier
			);
	}
	else
	{
		auto rco = URecipeCopierRCO::getRCO(smartSplitter);
		if (rco)
		{
			rco->ApplySmartSplitterInfo(
				smartSplitter,
				splitterRules,
				player,
				copier
				);
		}
	}

	RC_LOG_Display_Condition(*GetPathNameSafe(smartSplitter), TEXT(": Rules defined"));
}

void ARecipeCopierLogic::ApplySmartSplitterInfo_Server
(
	AFGBuildableSplitterSmart* smartSplitter,
	const TArray<FSplitterSortRule> splitterRules,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (!smartSplitter || !smartSplitter->HasAuthority())
	{
		return;
	}

	RC_LOG_Display_Condition(
		TEXT("Set Rules Impl: "),
		*GetPathNameSafe(smartSplitter),
		TEXT(" / "),
		splitterRules.Num(),
		TEXT(" / "),
		*GetPathNameSafe(player)
		)

	if (smartSplitter->IsA(programmableSplitterClass))
	{
		while (smartSplitter->GetNumSortRules() > 0)
		{
			smartSplitter->RemoveSortRuleAt(0);
		}

		for (auto rule : splitterRules)
		{
			smartSplitter->AddSortRule(rule);
		}
	}
	else
	{
		for (auto i = 0; i < smartSplitter->GetNumSortRules(); i++)
		{
			const FSplitterSortRule* rule = splitterRules.FindByPredicate([i](const FSplitterSortRule& rule) { return rule.OutputIndex == i; });
			if (rule)
			{
				smartSplitter->SetSortRuleAt(i, *rule);
			}
			else
			{
				smartSplitter->SetSortRuleAt(i, FSplitterSortRule(nullptr, i));
			}
		}
	}

	copier->ClearTargets();
}

void ARecipeCopierLogic::ApplyWidgetSignInfo
(
	AFGBuildableWidgetSign* widgetSign,
	const FLinearColor& foregroundColor,
	const FLinearColor& backgroundColor,
	const FLinearColor& auxiliaryColor,
	float emissive,
	float glossiness,
	const TMap<FString, FString>& texts,
	const TMap<FString, int32>& iconIds,
	int32 signCopyMode,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (widgetSign->HasAuthority())
	{
		ApplyWidgetSignInfo_Server(
			widgetSign,
			foregroundColor,
			backgroundColor,
			auxiliaryColor,
			emissive,
			glossiness,
			texts,
			iconIds,
			signCopyMode,
			player,
			copier
			);
	}
	else
	{
		auto rco = URecipeCopierRCO::getRCO(widgetSign);
		if (rco)
		{
			rco->ApplyWidgetSignInfo(
				widgetSign,
				foregroundColor,
				backgroundColor,
				auxiliaryColor,
				emissive,
				glossiness,
				texts,
				iconIds,
				signCopyMode,
				player,
				copier
				);
		}
	}
}

void ARecipeCopierLogic::ApplyWidgetSignInfo_Server
(
	AFGBuildableWidgetSign* widgetSign,
	const FLinearColor& foregroundColor,
	const FLinearColor& backgroundColor,
	const FLinearColor& auxiliaryColor,
	float emissive,
	float glossiness,
	const TMap<FString, FString>& texts,
	const TMap<FString, int32>& iconIds,
	int32 signCopyMode,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (!widgetSign->HasAuthority())
	{
		return;
	}

	FPrefabSignData signData;

	widgetSign->GetSignPrefabData(signData);

	if (Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Colors))
	{
		signData.ForegroundColor = foregroundColor;
		signData.BackgroundColor = backgroundColor;
		signData.AuxiliaryColor = auxiliaryColor;
	}
	if (Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_EmissiveAndGlossiness))
	{
		signData.Emissive = emissive;
		signData.Glossiness = glossiness;
	}

	if (Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Texts))
	{
		for (auto t : texts)
		{
			if (signData.TextElementData.Contains(t.Key))
			{
				signData.TextElementData[t.Key] = t.Value;
			}
		}
	}

	if (Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Icons))
	{
		for (auto t : iconIds)
		{
			if (signData.IconElementData.Contains(t.Key))
			{
				signData.IconElementData[t.Key] = t.Value;
			}
		}
	}

	if (Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Layout))
	{
	}

	widgetSign->SetPrefabSignData(signData);

	copier->ClearTargets();
}

void ARecipeCopierLogic::ApplyTrainInfo(AFGTrain* train, AFGCharacterPlayer* player, ARecipeCopierEquipment* copier)
{
	if (train->HasAuthority())
	{
		ApplyTrainInfo_Server(
			train,
			player,
			copier
			);
	}
	else
	{
		auto rco = URecipeCopierRCO::getRCO(train);
		if (rco)
		{
			rco->ApplyTrainInfo(
				train,
				player,
				copier
				);
		}
	}
}

void ARecipeCopierLogic::ApplyTrainInfo_Server(AFGTrain* train, AFGCharacterPlayer* player, ARecipeCopierEquipment* copier)
{
	if (!train || !train->HasAuthority())
	{
		return;
	}

	copier->ClearTargets();
}

bool ARecipeCopierLogic::CanProduceRecipe(AFGBuildableManufacturer* manufacturer, TSubclassOf<UFGRecipe> recipe)
{
	if (!manufacturer)
	{
		return false;
	}

	auto producedIn = UFGRecipe::GetProducedIn(recipe);

	for (auto i : producedIn)
	{
		if (manufacturer->IsA(i))
		{
			return true;
		}
	}

	return false;
}

void ARecipeCopierLogic::MoveItems
(
	UFGInventoryComponent* sourceInventoryComponent,
	UFGInventoryComponent* targetInventoryComponent,
	TSubclassOf<UFGItemDescriptor> item,
	int amount,
	bool dropExcess,
	AFGCharacterPlayer* player
)
{
	if (sourceInventoryComponent->GetOwner()->HasAuthority())
	{
		MoveItems_Server(
			sourceInventoryComponent,
			targetInventoryComponent,
			item,
			amount,
			dropExcess,
			player
			);
	}
	else
	{
		auto rco = URecipeCopierRCO::getRCO(sourceInventoryComponent->GetOwner());
		if (rco)
		{
			rco->MoveItems(
				sourceInventoryComponent,
				targetInventoryComponent,
				item,
				amount,
				dropExcess,
				player
				);
		}
	}
}

void ARecipeCopierLogic::MoveItems_Server
(
	UFGInventoryComponent* sourceInventoryComponent,
	UFGInventoryComponent* targetInventoryComponent,
	TSubclassOf<UFGItemDescriptor> item,
	int amount,
	bool dropExcess,
	AFGCharacterPlayer* player
)
{
	if (!sourceInventoryComponent || !targetInventoryComponent || !sourceInventoryComponent->GetOwner()->HasAuthority())
	{
		return;
	}

	amount = FMath::Min(amount, sourceInventoryComponent->GetNumItems(item));

	auto amountAdded = targetInventoryComponent->AddStack(FInventoryStack(amount, item), true);

	if (amountAdded > 0)
	{
		RC_LOG_Display_Condition(
			TEXT("Added "),
			amountAdded,
			(" "),
			*UFGItemDescriptor::GetItemName(item).ToString(),
			(" to "),
			*GetPathNameSafe(targetInventoryComponent)
			)

		sourceInventoryComponent->Remove(item, amountAdded);

		RC_LOG_Display_Condition(
			TEXT("Removed "),
			amountAdded,
			(" "),
			*UFGItemDescriptor::GetItemName(item).ToString(),
			(" from "),
			*GetPathNameSafe(sourceInventoryComponent)
			)
	}

	if (dropExcess && amountAdded < amount)
	{
		auto items = TArray<FInventoryStack>();
		items.Add(FInventoryStack(amount - amountAdded, item));

		TArray<class AFGItemPickup_Spawnable*> out_itemDrops;

		AFGItemPickup_Spawnable::CreateItemDropsInCylinder(
			sourceInventoryComponent->GetWorld(),
			items,
			player->GetActorLocation(),
			200,
			TArray<class AActor*>(),
			out_itemDrops
			);

		RC_LOG_Display_Condition(
			TEXT("Dropped "),
			amount - amountAdded,
			(" "),
			*UFGItemDescriptor::GetItemName(item).ToString()
			)
	}
}

void ARecipeCopierLogic::ConcatTexts(const TMap<FString, FString>& strings, FString& result, const FString& connector)
{
	result = TEXT("");

	for (auto s : strings)
	{
		if (!result.IsEmpty())
		{
			result += connector;
		}

		result += s.Key + " = " + s.Value;
	}
}

#ifndef OPTIMIZE
#pragma optimize( "", on)
#endif
