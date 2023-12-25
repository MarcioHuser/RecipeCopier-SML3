#pragma once

#include "Logging/LogMacros.h"

#include <sstream>

class CommaLog
{
public:
	template <typename T>
	inline CommaLog&
	operator,(const T& value)
	{
		wos << value;

		return *this;
	}

	inline CommaLog&
	operator,(const FString& value)
	{
		wos << *value;

		return *this;
	}

	inline CommaLog&
	operator,(const FText& value)
	{
		wos << *value.ToString();

		return *this;
	}

	std::wostringstream wos;
};

DECLARE_LOG_CATEGORY_EXTERN(LogRecipeCopier, Log, All)

#define RC_LOG_Verbosity(verbosity, first, ...) \
	{ \
		CommaLog l; \
		l, first, ##__VA_ARGS__; \
		UE_LOG(LogRecipeCopier, verbosity, TEXT("%s"), l.wos.str().c_str()) \
	}

#define RC_LOG_Log(first, ...) RC_LOG_Verbosity(Log, first, ##__VA_ARGS__)
#define RC_LOG_Display(first, ...) RC_LOG_Verbosity(Display, first, ##__VA_ARGS__)
#define RC_LOG_Warning(first, ...) RC_LOG_Verbosity(Warning, first, ##__VA_ARGS__)
#define RC_LOG_Error(first, ...) RC_LOG_Verbosity(Error, first, ##__VA_ARGS__)

#define IS_RC_LOG_LEVEL(level) (ARecipeCopierLogic::configuration.logLevel > 0 && ARecipeCopierLogic::configuration.logLevel >= static_cast<uint8>(level))

#define RC_LOG_Log_Condition(first, ...) if(IS_RC_LOG_LEVEL(ELogVerbosity::Log)) RC_LOG_Log(first, ##__VA_ARGS__)
#define RC_LOG_Display_Condition(first, ...) if(IS_RC_LOG_LEVEL(ELogVerbosity::Display)) RC_LOG_Display(first, ##__VA_ARGS__)
#define RC_LOG_Warning_Condition(first, ...) if(IS_RC_LOG_LEVEL(ELogVerbosity::Warning)) RC_LOG_Warning(first, ##__VA_ARGS__)
#define RC_LOG_Error_Condition(first, ...) if(IS_RC_LOG_LEVEL(ELogVerbosity::Error)) RC_LOG_Error(first, ##__VA_ARGS__)
