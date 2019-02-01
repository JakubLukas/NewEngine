#include "obj_import_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"
#include "core/asserts.h"
#include "core/string.h"


#include "core/os/os_utils.h"


namespace Veng
{

namespace Editor
{


static bool FileOpen(Path& path)
{
	os::FileDialogData data;
	data.filter = "Wavefront obj,.obj";
	string::Copy(data.fileName, "*.obj");
	
	bool result = ShowOpenFileDialog(data);
	if (result)
	{
		path.SetPath(data.fileName);
	}
	return result;
}

static bool FileSave(Path& path)
{
	os::FileDialogData data;
	data.filter = "Model file,.model";
	string::Copy(data.fileName, "*.model");

	bool result = ShowSaveFileDialog(data);
	if (result)
	{
		path.SetPath(data.fileName);
	}
	return result;
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
	ImGui::PushID("input");
	static char iPathBuffer[Path::MAX_LENGTH + 1];
	ImGui::InputText("##input", iPathBuffer, Path::MAX_LENGTH + 1);
	if (ImGui::BeginDragDropTarget())
	{
		const ImGuiPayload* data = ImGui::AcceptDragDropPayload("obj", ImGuiDragDropFlags_None);
		if (data != nullptr)
		{
			string::Copy(iPathBuffer, (const char*)data->Data);
		}
		ImGui::EndDragDropTarget();
	}
	ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		Path objFile;
		if(FileOpen(objFile))
			string::Copy(iPathBuffer, objFile.GetPath());
	}
	ImGui::SameLine();
	ImGui::Text("input");
	ImGui::PopID();
	
	ImGui::PushID("output");
	static char oPathBuffer[Path::MAX_LENGTH + 1];
	ImGui::InputText("##output", oPathBuffer, Path::MAX_LENGTH + 1);
	ImGui::SameLine();
	if (ImGui::Button("Browse"))
	{
		Path objFile;
		if (FileSave(objFile))
			string::Copy(oPathBuffer, objFile.GetPath());
	}
	ImGui::SameLine();
	ImGui::Text("output");
	ImGui::PopID();
}


REGISTER_WIDGET(obj_import)
{
	return NEW_OBJECT(allocator, ObjImportWidget)(allocator);
}


}

}