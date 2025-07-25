#pragma once

#include "Containers/UnrealString.h"
#include "UObject/ReflectedTypeAccessors.h"
#include "Templates/IsUEnumClass.h"

namespace AlsEnumUtility
{
	template <typename EnumType> requires std::is_enum_v<EnumType>
	int32 GetIndexByValue(const EnumType Value)
	{
		return StaticEnum<EnumType>()->GetIndexByValue(static_cast<int64>(Value));
	}

	template <typename EnumType> requires std::is_enum_v<EnumType>
	FString GetNameStringByValue(const EnumType Value)
	{
		return StaticEnum<EnumType>()->GetNameStringByValue(static_cast<int64>(Value));
	}
}
