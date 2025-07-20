#pragma once

#include "Misc/AssertionMacros.h"

#define ALS_STRINGIFY_IMPLEMENTATION(Value) #Value

#define ALS_STRINGIFY(Value) ALS_STRINGIFY_IMPLEMENTATION(Value)

#define ALS_GET_TYPE_STRING(Type) \
	((void) sizeof UEAsserts_Private::GetMemberNameCheckedJunk(static_cast<Type*>(nullptr)), TEXTVIEW(#Type))

// A lightweight version of the ensure() macro that doesn't generate a C++ call stack and doesn't send a
// crash report, because it doesn't happen instantly and causes the editor to freeze, which can be annoying.

#if DO_ENSURE && !USING_CODE_ANALYSIS

#define ALS_ENSURE(Expression) ensure(Expression)
#define ALS_ENSURE_MESSAGE(Expression, Format, ...) ensureMsgf(Expression, Format, ##__VA_ARGS__)
#define ALS_ENSURE_ALWAYS(Expression) ensureAlways(Expression)
#define ALS_ENSURE_ALWAYS_MESSAGE(Expression, Format, ...) ensureAlwaysMsgf(Expression, Format, ##__VA_ARGS__)

#else

#define ALS_ENSURE(Expression) (Expression)
#define ALS_ENSURE_MESSAGE(Expression, Format, ...) (Expression)
#define ALS_ENSURE_ALWAYS(Expression) (Expression)
#define ALS_ENSURE_ALWAYS_MESSAGE(Expression, Format, ...) (Expression)

#endif
