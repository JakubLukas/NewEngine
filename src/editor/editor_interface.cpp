#include "editor_interface.h"

#include "core/asserts.h"
#include "core/string.h"
#include "core/engine.h"
#include "core/resource/resource_manager.h"
#include "editor/event_queue.h"

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"

#include "core/logs.h"/////////////////////// TEMP


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


EditorInterface::EditorInterface(Engine& engine, Editor::EventQueue& message_queue)
	: m_engine(engine)
	, m_queue(message_queue)
{}


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

bool EditorInterface::EditString(const char* name, char* buffer, size_t bufferLength, EditFlags flags)
{
	return ImGui::InputText("path", buffer, bufferLength, ResolveFlags(flags));
}

bool EditorInterface::EditColor(const char* name, Color& color, EditFlags flags)
{
	float rgba[4] = {
		(color.abgr & 0xFF) / 255.0f,
		(color.abgr >> 8 & 0xFF) / 255.0f,
		(color.abgr >> 16 & 0xFF) / 255.0f,
		(color.abgr >> 24 & 0xFF) / 255.0f
	};
	if (ImGui::ColorEdit4(name, rgba, ImGuiColorEditFlags_None))
	{
		color.abgr = u32(rgba[3] * 255) << 24
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

bool EditorInterface::EditResource(const char* name, ResourceType type, resourceHandle& handle, EditFlags flags)
{
	ResourceManager* manager = m_engine.GetResourceManager(type);
	Resource* resource = manager->GetResource(handle);
	bool pathChanged = false;
	char pathBuffer[Path::BUFFER_LENGTH] = { 0 };
	if (handle != INVALID_RESOURCE_HANDLE)
		memory::Copy(pathBuffer, resource->GetPath().GetPath(), Path::BUFFER_LENGTH);

	if (EditString("path", pathBuffer, Path::BUFFER_LENGTH))
		pathChanged = true;
	for (size_t i = 0; i < manager->GetSupportedFileExtCount(); ++i)
	{
		if (ImGui::BeginDragDropTarget())
		{
			const ImGuiPayload* data = ImGui::AcceptDragDropPayload(manager->GetSupportedFileExt()[i], ImGuiDragDropFlags_None);
			if (data != nullptr)
			{
				string::Copy(pathBuffer, (char*)data->Data);
				pathChanged = true;
			}
			ImGui::EndDragDropTarget();
		}
	}
	if (pathChanged)
	{
		Path newPath(pathBuffer);
		resourceHandle newHandle = manager->Load(newPath);
		resourceHandle oldHandle = handle;
		handle = newHandle;
		if (oldHandle != INVALID_RESOURCE_HANDLE)
			manager->Unload(oldHandle);
	}
	if (ImGui::Button("Open in editor"))
	{
		m_queue.PushEvent(Editor::EventSelectResource{ type, handle });
	}
	return pathChanged;
}


}