#pragma once

#include "core/int.h"

namespace Veng
{


class EditorInterface
{
public:
	typedef u32 EditFlags;
	enum EditFlagBits : EditFlags
	{
		EditFlag_None = 0,
		EditFlag_ReadOnly = 1 << 0,
	};

	bool EditI32(const char* name, i32& value, EditFlags flags = EditFlag_None);
	bool EditU32(const char* name, u32& value, EditFlags flags = EditFlag_None);
	bool EditI64(const char* name, i64& value, EditFlags flags = EditFlag_None);
	bool EditU64(const char* name, u64& value, EditFlags flags = EditFlag_None);
	bool EditFloat(const char* name, float& value, EditFlags flags = EditFlag_None);
	bool EditPointer(const char* name, void* value, EditFlags flags = EditFlag_None);
	bool EditColor(const char* name, u32& abgrColor, EditFlags flags = EditFlag_None);
	bool EditEnum(const char* name, u32& idx, const char* values[], size_t count, EditFlags flags = EditFlag_None);
};


}