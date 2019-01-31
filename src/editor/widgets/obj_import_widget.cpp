#include "obj_import_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"
#include "core/asserts.h"
#include "core/string.h"


#include "core/os/win/simple_windows.h"


namespace Veng
{

namespace Editor
{


void BasicFileOpen()
{
	OPENFILENAME dialogInfo;
	GetOpenFileName(&dialogInfo);
}


ObjImportWidget::ObjImportWidget(IAllocator& allocator)
	: m_allocator(allocator)
{}


ObjImportWidget::~ObjImportWidget()
{}


void ObjImportWidget::Init(Engine& engine)
{}


void ObjImportWidget::Deinit()
{}


void ObjImportWidget::Update(EventQueue& queue)
{}


void ObjImportWidget::RenderInternal(EventQueue& queue)
{
	static char pathBuffer[Path::MAX_LENGTH + 1];
	ImGui::InputText("##input", pathBuffer, Path::MAX_LENGTH + 1);
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* data = ImGui::AcceptDragDropPayload("obj", ImGuiDragDropFlags_None);
		if (data != nullptr)
		{
			string::Copy(pathBuffer, (const char*)data->Data);
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{

	}
	ImGui::SameLine();
	ImGui::Text("input");
	
	ImGui::InputText("##output", pathBuffer, Path::MAX_LENGTH + 1);
	ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{

	}
	ImGui::SameLine();
	ImGui::Text("output");
}


REGISTER_WIDGET(obj_import)
{
	return NEW_OBJECT(allocator, ObjImportWidget)(allocator);
}


}

}