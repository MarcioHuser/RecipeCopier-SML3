// ReSharper disable CppUE4CodingStandardNamingViolationWarning
// ReSharper disable CommentTypo
// ReSharper disable IdentifierTypo

#include "Logic/RecipeCopierLogic.h"

#include "FGCharacterPlayer.h"
#include "FGItemPickup_Spawnable.h"
#include "FGTrain.h"
#include "FGTrainStationIdentifier.h"
#include "RecipeCopierEquipment.h"
#include "RecipeCopierRCO.h"
#include "Buildables/FGBuildableLightsControlPanel.h"
#include "Buildables/FGBuildableManufacturer.h"
#include "Buildables/FGBuildablePipelinePump.h"
#include "Buildables/FGBuildableWidgetSign.h"
#include "Reflection/BlueprintReflectionLibrary.h"
#include "Util/RCOptimize.h"
#include "Util/RCLogging.h"

#include "Resources/FGItemDescriptor.h"
#include "Util/MapHelpers.h"

#ifndef OPTIMIZE
#pragma optimize("", off )
#endif

ARecipeCopierLogic* ARecipeCopierLogic::singleton = nullptr;
FRecipeCopier_ConfigStruct ARecipeCopierLogic::configuration;
TSubclassOf<UFGItemDescriptor> ARecipeCopierLogic::shardItemDescriptor;
TSubclassOf<AFGBuildableSplitterSmart> ARecipeCopierLogic::programmableSplitterClass;
TSubclassOf<AFGBuildablePipelinePump> ARecipeCopierLogic::valveClass;

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
	TSubclassOf<AFGBuildableSplitterSmart> in_programmableSplitterClass,
	TSubclassOf<AFGBuildablePipelinePump> in_valveClass
)
{
	shardItemDescriptor = in_shardItemDescriptor;
	programmableSplitterClass = in_programmableSplitterClass;
	valveClass = in_valveClass;
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

int ARecipeCopierLogic::GetLogLevelRC()
{
	return configuration.logLevel;
}

bool ARecipeCopierLogic::IsLogLevelDisplayRC()
{
	return GetLogLevelRC() > 4;
}

// void ARecipeCopierLogic::DumpUnknownClass(UObject* obj)
// {
// 	if (IS_RC_LOG_LEVEL(ELogVerbosity::Log))
// 	{
// 		RC_LOG_Display(TEXT("Object "), *obj->GetPathName());
// 		RC_LOG_Display(TEXT("Class "), *obj->GetClass()->GetPathName());
//
// 		for (auto cls = obj->GetClass()->GetSuperClass(); cls && cls != AActor::StaticClass(); cls = cls->GetSuperClass())
// 		{
// 			RC_LOG_Display(TEXT("    - Super: "), *cls->GetPathName());
// 		}
//
// 		RC_LOG_Display(TEXT("Properties"));
//
// 		for (TFieldIterator<FProperty> property(obj->GetClass()); property; ++property)
// 		{
// 			auto floatProperty = CastField<FFloatProperty>(*property);
// 			auto doubleProperty = CastField<FDoubleProperty>(*property);
// 			auto intProperty = CastField<FIntProperty>(*property);
// 			auto boolProperty = CastField<FBoolProperty>(*property);
// 			auto structProperty = CastField<FStructProperty>(*property);
// 			auto strProperty = CastField<FStrProperty>(*property);
// 			auto textProperty = CastField<FTextProperty>(*property);
// 			auto classProperty = CastField<FClassProperty>(*property);
// 			auto objectProperty = CastField<FObjectProperty>(*property);
// 			auto weakObjectProperty = CastField<FWeakObjectProperty>(*property);
// 			auto arrayProperty = CastField<FArrayProperty>(*property);
// 			auto byteProperty = CastField<FByteProperty>(*property);
// 			auto nameProperty = CastField<FNameProperty>(*property);
// 			auto enumProperty = CastField<FEnumProperty>(*property);
//
// 			FString cppType;
//
// 			if (floatProperty ||
// 				doubleProperty ||
// 				intProperty ||
// 				boolProperty ||
// 				structProperty ||
// 				strProperty ||
// 				textProperty ||
// 				classProperty ||
// 				objectProperty && objectProperty->PropertyClass ||
// 				weakObjectProperty && weakObjectProperty->PropertyClass ||
// 				arrayProperty ||
// 				byteProperty ||
// 				nameProperty ||
// 				enumProperty
// 				)
// 			{
// 				cppType = property->GetCPPType();
// 			}
// 			else
// 			{
// 				cppType = TEXT("<<Unknown>>");
// 			}
//
// 			RC_LOG_Display(
// 				TEXT("    - "),
// 				*property->GetName(),
// 				TEXT(" ("),
// 				*cppType,
// 				TEXT(" / "),
// 				*property->GetClass()->GetName(),
// 				TEXT(" / RepIndex = "),
// 				property->RepIndex,
// 				TEXT(")")
// 				);
//
// 			if (floatProperty)
// 			{
// 				RC_LOG_Display(TEXT("        = "), floatProperty->GetPropertyValue_InContainer(obj));
// 			}
// 			else if (doubleProperty)
// 			{
// 				RC_LOG_Display(TEXT("        = "), doubleProperty->GetPropertyValue_InContainer(obj));
// 			}
// 			else if (intProperty)
// 			{
// 				RC_LOG_Display(TEXT("        = "), intProperty->GetPropertyValue_InContainer(obj));
// 			}
// 			else if (byteProperty)
// 			{
// 				RC_LOG_Display(TEXT("        = "), byteProperty->GetPropertyValue_InContainer(obj));
// 			}
// 			else if (boolProperty)
// 			{
// 				RC_LOG_Display(TEXT("        = "), boolProperty->GetPropertyValue_InContainer(obj) ? TEXT("true") : TEXT("false"));
// 			}
// 			else if (structProperty && cppType == TEXT("FFactoryTickFunction"))
// 			{
// 				auto factoryTick = structProperty->ContainerPtrToValuePtr<FFactoryTickFunction>(obj);
// 				if (factoryTick)
// 				{
// 					RC_LOG_Display(TEXT("        - Tick Interval = "), factoryTick->TickInterval);
// 				}
// 			}
// 			else if (strProperty)
// 			{
// 				RC_LOG_Display(TEXT("        = "), *strProperty->GetPropertyValue_InContainer(obj));
// 			}
// 			else if (textProperty)
// 			{
// 				RC_LOG_Display(TEXT("        = "), *textProperty->GetPropertyValue_InContainer(obj).ToString());
// 			}
// 			else if (nameProperty)
// 			{
// 				RC_LOG_Display(TEXT("        = "), *nameProperty->GetPropertyValue_InContainer(obj).ToString());
// 			}
// 			else if (classProperty)
// 			{
// 				RC_LOG_Display(TEXT("        = "), *GetNameSafe(classProperty->GetPropertyValue_InContainer(obj)));
// 			}
// 			else if (objectProperty)
// 			{
// 				if (cppType == TEXT("UWidgetComponent*"))
// 				{
// 					auto widgetComponent = GetValid(objectProperty->ContainerPtrToValuePtr<UWidgetComponent>(obj));
// 					if (widgetComponent)
// 					{
// 						RC_LOG_Display(TEXT("            - "), *GetPathNameSafe(widgetComponent->GetClass()));
// 					}
// 				}
// 				else if (cppType == TEXT("AActor*"))
// 				{
// 					AActor* actor = nullptr;
//
// 					if (objectProperty->GetName() == TEXT("mBuildEffectInstignator"))
// 					{
// 						auto actorPtr = objectProperty->ContainerPtrToValuePtr<AActor*>(obj);
// 						if (actorPtr)
// 						{
// 							actor = *actorPtr;
// 						}
// 					}
// 					else
// 					{
// 						actor = objectProperty->ContainerPtrToValuePtr<AActor>(obj);
// 					}
//
// 					actor = GetValid(actor);
//
// 					if (actor)
// 					{
// 						RC_LOG_Display(TEXT("            - "), *GetPathNameSafe(actor));
// 					}
// 				}
// 			}
// 			else if (arrayProperty)
// 			{
// 				FScriptArrayHelper arrayHelper(arrayProperty, arrayProperty->ContainerPtrToValuePtr<void>(obj));
//
// 				RC_LOG_Display(TEXT("        - CPPTypeForwardDeclaration = "), arrayProperty->GetCPPTypeForwardDeclaration());
// 				RC_LOG_Display(TEXT("        - Num = "), arrayHelper.Num());
//
// 				auto arrayObjectProperty = CastField<FObjectProperty>(arrayProperty->Inner);
// 				auto arrayWeakObjectProperty = CastField<FWeakObjectProperty>(arrayProperty->Inner);
//
// 				if (arrayObjectProperty)
// 				{
// 					for (auto x = 0; x < arrayHelper.Num(); x++)
// 					{
// 						void* ObjectContainer = arrayHelper.GetRawPtr(x);
// 						auto Object = arrayObjectProperty->GetObjectPropertyValue(ObjectContainer);
// 						RC_LOG_Display(
// 							TEXT("            - "),
// 							x,
// 							TEXT(" = "),
// 							*GetPathNameSafe(Object),
// 							TEXT(" ("),
// 							*GetPathNameSafe(Object ? Object->GetClass() : nullptr),
// 							TEXT(" )")
// 							);
// 					}
// 				}
// 				else if (arrayWeakObjectProperty)
// 				{
// 					for (auto x = 0; x < arrayHelper.Num(); x++)
// 					{
// 						void* ObjectContainer = arrayHelper.GetRawPtr(x);
// 						auto Object = arrayWeakObjectProperty->GetObjectPropertyValue(ObjectContainer);
//
// 						RC_LOG_Display(
// 							TEXT("            - "),
// 							x,
// 							TEXT(" = "),
// 							*GetPathNameSafe(Object),
// 							TEXT(" ("),
// 							*GetPathNameSafe(Object ? Object->GetClass() : nullptr),
// 							TEXT(" )")
// 							);
// 					}
// 				}
// 			}
// 			else if (enumProperty)
// 			{
// 				const auto objReflection = UBlueprintReflectionLibrary::ReflectObject(obj);
//
// 				const auto reflectedValue = objReflection.GetEnumProperty(FName(property->GetName()));
//
// 				auto currentValue = reflectedValue.GetCurrentValue();
// 				auto enumType = enumProperty->GetEnum();
//
// 				RC_LOG_Display(
// 					TEXT("        = "),
// 					currentValue,
// 					TEXT(" ("),
// 					*(enumType ? enumType->GetNameByValue(currentValue) : NAME_None).ToString(),
// 					TEXT(")")
// 					);
// 			}
// 		}
//
// 		RC_LOG_Display(TEXT("===="));
// 	}
// }

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
	TSubclassOf<class UFGSignPrefabWidget> prefabLayout,
	TSubclassOf<class UFGSignTypeDescriptor> signTypeDesc,
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
			prefabLayout,
			signTypeDesc,
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
			TArray<FString> textKeys;
			TArray<FString> textValues;

			TArray<FString> iconIdKeys;
			TArray<int32> iconIdValues;

			MapToArrays(texts, textKeys, textValues);
			MapToArrays(iconIds, iconIdKeys, iconIdValues);

			rco->ApplyWidgetSignInfo(
				widgetSign,
				foregroundColor,
				backgroundColor,
				auxiliaryColor,
				emissive,
				glossiness,
				textKeys,
				textValues,
				iconIdKeys,
				iconIdValues,
				prefabLayout,
				signTypeDesc,
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
	TSubclassOf<class UFGSignPrefabWidget> prefabLayout,
	TSubclassOf<class UFGSignTypeDescriptor> signTypeDesc,
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

	if (Has_ESignCopyModeType(signCopyMode, ESignCopyModeType::SCMT_Layout) &&
		signData.SignTypeDesc == signTypeDesc)
	{
		signData.PrefabLayout = prefabLayout;
	}

	widgetSign->SetPrefabSignData(signData);

	copier->ClearTargets();
}

void ARecipeCopierLogic::ApplyTrainInfo
(
	AFGTrain* train,
	const TArray<FTimeTableStop>& trainStops,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (train->HasAuthority())
	{
		ApplyTrainInfo_Server(
			train,
			trainStops,
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
				trainStops,
				player,
				copier
				);
		}
	}
}

void ARecipeCopierLogic::ApplyTrainInfo_Server
(
	AFGTrain* train,
	const TArray<FTimeTableStop>& trainStops,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (!train || !train->HasAuthority())
	{
		return;
	}

	auto timetable = train->GetTimeTable();
	if (!timetable)
	{
		timetable = train->NewTimeTable();
	}

	timetable->SetStops(trainStops);
	timetable->PurgeInvalidStops();

	copier->ClearTargets();
}

void ARecipeCopierLogic::ApplyLightsControlPanel
(
	class AFGBuildableLightsControlPanel* lightsControlPanel,
	const FLightSourceControlData& lightSourceControlData,
	bool isLightEnabled,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (lightsControlPanel->HasAuthority())
	{
		ApplyLightsControlPanel_Server(
			lightsControlPanel,
			lightSourceControlData,
			isLightEnabled,
			player,
			copier
			);
	}
	else
	{
		auto rco = URecipeCopierRCO::getRCO(lightsControlPanel);
		if (rco)
		{
			rco->ApplyLightsControlPanel(
				lightsControlPanel,
				lightSourceControlData,
				isLightEnabled,
				player,
				copier
				);
		}
	}
}

void ARecipeCopierLogic::ApplyLightsControlPanel_Server
(
	class AFGBuildableLightsControlPanel* lightsControlPanel,
	const FLightSourceControlData& lightSourceControlData,
	bool isLightEnabled,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (!lightsControlPanel || !lightsControlPanel->HasAuthority())
	{
		return;
	}

	auto lightControlData = lightsControlPanel->GetLightControlData();

	lightControlData.ColorSlotIndex = lightSourceControlData.ColorSlotIndex;
	lightControlData.Intensity = lightSourceControlData.Intensity;
	lightControlData.IsTimeOfDayAware = lightSourceControlData.IsTimeOfDayAware;

	lightsControlPanel->SetLightControlData(lightControlData);
	lightsControlPanel->SetLightDataOnControlledLights(lightControlData);

	lightsControlPanel->SetLightEnabled(isLightEnabled);

	/*if (lightsControlPanel->OnLightControlPanelStateChanged.IsBound())
	{
		lightsControlPanel->OnLightControlPanelStateChanged.Broadcast(isLightEnabled);
	}*/

	copier->ClearTargets();
}

void ARecipeCopierLogic::ApplyValve
(
	class AFGBuildablePipelinePump* valve,
	float userFlowLimit,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (valve->HasAuthority())
	{
		ApplyValve_Server(
			valve,
			userFlowLimit,
			player,
			copier
			);
	}
	else
	{
		auto rco = URecipeCopierRCO::getRCO(valve);
		if (rco)
		{
			rco->ApplyValve(
				valve,
				userFlowLimit,
				player,
				copier
				);
		}
	}
}

void ARecipeCopierLogic::ApplyValve_Server
(
	class AFGBuildablePipelinePump* valve,
	float userFlowLimit,
	AFGCharacterPlayer* player,
	ARecipeCopierEquipment* copier
)
{
	if (!valve || !valve->HasAuthority())
	{
		return;
	}

	valve->SetUserFlowLimit(userFlowLimit);

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

template <typename C, typename V>
void executeIfBound(C callback, UPanelWidget* container, const V& value)
{
	if (callback.IsBound())
	{
		callback.Execute(container, value);
	}
}

void ARecipeCopierLogic::SetTimetable
(
	UPanelWidget* containerWidget,
	FAppendLabel appendLabelCallback,
	FAppendItemIcon appendItemIconCallback,
	const TArray<FTimeTableStop>& trainStops
)
{
	containerWidget->ClearChildren();

	auto firstStop = true;
	for (auto trainStop : trainStops)
	{
		if (!trainStop.Station)
		{
			continue;
		}

		if (!firstStop)
		{
			executeIfBound(appendLabelCallback, containerWidget,TEXT(" » "));
		}
		else
		{
			firstStop = false;
		}

		FString leftStationName;
		FString rightStationName;
		auto stationName = trainStop.Station->GetStationName().ToString();
		while (stationName.Split(TEXT(" "), &leftStationName, &rightStationName))
		{
			executeIfBound(appendLabelCallback, containerWidget, leftStationName + TEXT(" "));

			stationName = rightStationName;
		}
		executeIfBound(appendLabelCallback, containerWidget, stationName + TEXT("; "));

		// executeIfBound(appendLabelCallback, containerWidget, trainStop.Station->GetStationName().ToString() + TEXT("; "));
		executeIfBound(appendLabelCallback, containerWidget, TEXT("Wait: "));
		executeIfBound(appendLabelCallback, containerWidget, FString::SanitizeFloat(trainStop.DockingRuleSet.DockForDuration, 0));
		executeIfBound(appendLabelCallback, containerWidget,TEXT(" "));
		executeIfBound(
			appendLabelCallback,
			containerWidget,
			trainStop.DockingRuleSet.DockingDefinition == ETrainDockingDefinition::TDD_FullyLoadUnload
				? TEXT("Full; ")
				: TEXT("Once; ")
			);

		if (trainStop.DockingRuleSet.LoadFilterDescriptors.Num())
		{
			executeIfBound(appendLabelCallback, containerWidget,TEXT("Load: "));

			auto firstFilter = true;
			for (auto filter : trainStop.DockingRuleSet.LoadFilterDescriptors)
			{
				executeIfBound(appendItemIconCallback, containerWidget, filter);
			}

			executeIfBound(appendLabelCallback, containerWidget,TEXT("; "));
		}

		if (trainStop.DockingRuleSet.LoadFilterDescriptors.Num())
		{
			executeIfBound(appendLabelCallback, containerWidget,TEXT("Unload: "));

			auto firstFilter = true;
			for (auto filter : trainStop.DockingRuleSet.UnloadFilterDescriptors)
			{
				executeIfBound(appendItemIconCallback, containerWidget, filter);
			}

			executeIfBound(appendLabelCallback, containerWidget,TEXT("; "));
		}
	}
}

#ifndef OPTIMIZE
#pragma optimize("", on)
#endif
