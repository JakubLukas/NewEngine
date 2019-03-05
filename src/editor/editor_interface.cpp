#include "editor_interface.h"

#include "core/asserts.h"

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"


namespace Veng
{


static ImGuiInputTextFlags ResolveFlags(EditorInterface::EditFlags flags)
{
	ImGuiInputTextFlags imFlags = ImGuiInputTextFlags_None
		| ImGuiInputTextFlags_AutoSelectAll
		| ImGuiInputTextFlags_EnterReturnsTrue;
	if (flags & EditorInterface::EditFlag_ReadOnly)
		imFlags |= ImGuiInputTextFlags_ReadOnly;

	return imFlags;
}


bool EditorInterface::EditI32(const char* name, i32& value, EditFlags flags)
{
	return ImGui::InputScalar(name, ImGuiDataType_S32, &value, 0, 0, 0, ResolveFlags(flags));
}

bool EditorInterface::EditU32(const char* name, u32& value, EditFlags flags)
{
	return ImGui::InputScalar(name, ImGuiDataType_U32, &value, 0, 0, 0, ResolveFlags(flags));
}

bool EditorInterface::EditI64(const char* name, i64& value, EditFlags flags)
{
	return ImGui::InputScalar(name, ImGuiDataType_S64, &value, 0, 0, 0, ResolveFlags(flags));
}

bool EditorInterface::EditU64(const char* name, u64& value, EditFlags flags)
{
	return ImGui::InputScalar(name, ImGuiDataType_U64, &value, 0, 0, 0, ResolveFlags(flags));
}

bool EditorInterface::EditFloat(const char* name, float& value, EditFlags flags)
{
	return ImGui::InputScalar(name, ImGuiDataType_Float, &value, 0, 0, 0, ResolveFlags(flags));
}

bool EditorInterface::EditPointer(const char* name, void* value, EditFlags flags)
{
	if (sizeof(value) == 4)
		return ImGui::InputScalar(name, ImGuiDataType_U32, value, 0, 0, 0, ResolveFlags(flags));
	else if (sizeof(value) == 8)
		return ImGui::InputScalar(name, ImGuiDataType_U64, value, 0, 0, 0, ResolveFlags(flags));
	else
		ASSERT2(false, "Invalid size of pointer");
	return false;
}

bool EditorInterface::EditColor(const char* name, u32& abgrColor, EditFlags flags)
{
	float rgba[4] = {
		(abgrColor >> 24 & 0xFF) / 255.0f,
		(abgrColor >> 16 & 0xFF) / 255.0f,
		(abgrColor >> 8 & 0xFF) / 255.0f,
		(abgrColor & 0xFF) / 255.0f
	};
	if (ImGui::ColorEdit4(name, rgba, ImGuiColorEditFlags_None))
	{
		abgrColor = u32(rgba[3] * 255) << 24
			| u32(rgba[2] * 255) << 16
			| u32(rgba[1] * 255) << 8
			| u32(rgba[0] * 255);
		return true;
	}
	else
	{
		return false;
	}
}

bool EditorInterface::EditEnum(const char* name, u32& idx, const char* values[], size_t count, EditFlags flags)
{
	bool changed = false;
	if (ImGui::BeginCombo(name, values[idx], ImGuiComboFlags_None))
	{
		for (int i = 0; i < count; ++i)
		{
			if (ImGui::Selectable(values[i], (i == idx), ImGuiSelectableFlags_None))
			{
				idx = i;
				changed = true;
			}
		}
		ImGui::EndCombo();
	}
	return changed;
}


}