#pragma once

#include "SignCopyModeType.generated.h"

UENUM(Blueprintable, Meta = (Bitflags, UseEnumValuesAsMaskValuesInEditor = "true"))
enum class ESignCopyModeType: uint8
{
	SCMT_None = 0 UMETA(DisplayName = "None"),
	
	SCMT_Colors = 1 << 0 UMETA(DisplayName = "Colors"),
	SCMT_EmissiveAndGlossiness = 1 << 1 UMETA(DisplayName = "Emissive & Glossiness"),
	SCMT_Texts = 1 << 2 UMETA(DisplayName = "Texts"),
	SCMT_Icons = 1 << 3 UMETA(DisplayName = "Icons"),
	SCMT_Layout = 1 << 4 UMETA(DisplayName = "Layout"),
	//
	SCMT_All = 0xff UMETA(DisplayName = "Include All"),
};

ENUM_CLASS_FLAGS(ESignCopyModeType);

#define TO_ESignCopyModeType(Enum) static_cast<uint32>(Enum)
#define Has_ESignCopyModeType(Value, Enum) ((static_cast<uint32>(Value) & static_cast<uint32>(Enum)) == static_cast<uint32>(Enum))
#define Add_ESignCopyModeType(Value, Enum) (static_cast<uint32>(Value) | static_cast<uint32>(Enum))
#define Remove_ESignCopyModeType(Value, Enum) (static_cast<uint32>(Value) & ~static_cast<uint32>(Enum))
#define Toggle_ESignCopyModeType(Value, Enum) (static_cast<uint32>(Value) ^ static_cast<uint32>(Enum))
