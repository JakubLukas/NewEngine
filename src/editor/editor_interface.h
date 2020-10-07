#pragma once

#include "core/int.h"
#include "core/color.h"
#include "core/entity.h"
#include "core/system.h"
#include "core/resource/resource.h"

namespace Veng
{

class Engine;
namespace Editor { class EventQueue; }


class EditorInterface
{
public:
	typedef u32 EditFlags;
	enum EditFlagBits : EditFlags
	{
		EditFlag_None = 0,
		EditFlag_ReadOnly = 1 << 0,
	};

public:
	EditorInterface(Engine& engine, Editor::EventQueue& message_queue);

	bool EditI32(const char* name, i32& value, EditFlags flags = EditFlag_None);
	bool EditU32(const char* name, u32& value, EditFlags flags = EditFlag_None);
	bool EditI64(const char* name, i64& value, EditFlags flags = EditFlag_None);
	bool EditU64(const char* name, u64& value, EditFlags flags = EditFlag_None);
	bool EditFloat(const char* name, float& value, EditFlags flags = EditFlag_None);
	bool EditString(const char* name, char* buffer, size_t bufferLength, EditFlags flags = EditFlag_None);
	bool EditColor(const char* name, Color& color, EditFlags flags = EditFlag_None);
	bool EditEnum(const char* name, u32& idx, const char* values[], size_t count, EditFlags flags = EditFlag_None);
	bool EditResource(const char* name, ResourceType type, resourceHandle& handle, EditFlags flags = EditFlag_None);

private:
	Engine& m_engine;
	Editor::EventQueue& m_queue;
};


}