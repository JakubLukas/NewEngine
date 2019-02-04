#include "obj_import_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"

#include "core/iallocator.h"
#include "core/asserts.h"
#include "core/logs.h"
#include "core/string.h"
#include "core/containers/array.h"
#include "core/math/vector.h"


#include "core/os/os_utils.h"
#include "core/file/file.h"

#include <cstdio>


namespace Veng
{

namespace Editor
{


static bool FileOpenDialog(Path& path)
{
	os::FileDialogData data;
	data.filter = "Wavefront obj,.obj";
	os::GetWorkingDir(data.fileName, Path::MAX_LENGTH);
	string::Copy(data.fileName + string::Length(data.fileName), "/*.obj");
	os::PathToNativePath(data.fileName);
	
	bool result = ShowOpenFileDialog(data);
	if (result)
	{
		os::PathToEnginePath(data.fileName);
		path.SetPath(data.fileName);
	}
	return result;
}

static bool FileSaveDialog(Path& path)
{
	os::FileDialogData data;
	data.filter = "Model file,.model";
	os::GetWorkingDir(data.fileName, Path::MAX_LENGTH);
	string::Copy(data.fileName + string::Length(data.fileName), "/*.model");
	os::PathToNativePath(data.fileName);

	bool result = ShowSaveFileDialog(data);
	if (result)
	{
		os::PathToEnginePath(data.fileName);
		path.SetPath(data.fileName);
	}
	return result;
}

static bool ConvertObj(IAllocator& allocator, const Path& inPath, const Path& outPath, char error[64])
{
	const FileMode fMode{
		FileMode::Access::Read,
		FileMode::ShareMode::ShareRead,
		FileMode::CreationDisposition::OpenExisting,
		FileMode::FlagNone
	};
	nativeFileHandle fh;
	if(!FS::OpenFileSync(fh, inPath, fMode))
	{
		ASSERT(false);
		string::Copy(error, "Could not open file");
		return false;
	}

	size_t fSize = FS::GetFileSize(fh);
	char* fileBuffer = (char*)allocator.Allocate(fSize, alignof(char));
	size_t charsRead = 0;
	if(!FS::ReadFileSync(fh, 0, fileBuffer, fSize, charsRead))
	{
		ASSERT(false);
		string::Copy(error, "Could not read from file");
		return false;
	}
	ASSERT2(fSize == charsRead, "Read size is not equal file size");
	ASSERT2(FS::CloseFileSync(fh), "Closing of file failed");

	Array<Vector3> vertices(allocator);
	Array<Vector2> uvs(allocator);
	Array<Vector2> normals(allocator);

	char lineBuffer[128];
	int charsUsed = 0;
	while(sscanf_s(fileBuffer, "%s%n", lineBuffer, 128, &charsUsed) > 0)
	{
		fileBuffer += charsUsed;
		if(string::Compare(lineBuffer, "v") == 0)
		{
			Vector3& v = vertices.PushBack();
			sscanf_s(lineBuffer + 1, "%f %f %f\n%n", &v.x, &v.y, &v.z, &charsUsed);
		}
		else if(string::Compare(lineBuffer, "vt") == 0)
		{
			Vector2& v = uvs.PushBack();
			sscanf_s(lineBuffer + 2, "%f %f\n%n", &v.x, &v.y, &charsUsed);
		}
		else if(string::Compare(lineBuffer, "vn") == 0)
		{
			Vector2& v = normals.PushBack();
			sscanf_s(lineBuffer + 2, "%f %f\n%n", &v.x, &v.y, &charsUsed);
		}
		else if(string::Compare(lineBuffer, "f") == 0)
		{
			u32 vIdx[3];
			u32 vtIdx[3];
			u32 vnIdx[3];
			sscanf_s(lineBuffer + 1, "%d/%d/%d %d/%d/%d %d/%d/%d\n%n", &vIdx[0], &vtIdx[0], &vnIdx[0], &vIdx[1], &vtIdx[1], &vIdx[1], &vnIdx[2], &vtIdx[2], &vnIdx[2], &charsUsed);
		}
		if unread line ,skip it
		fileBuffer += charsUsed;
	}


	return false;
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
		if(FileOpenDialog(objFile))
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
		if (FileSaveDialog(objFile))
			string::Copy(oPathBuffer, objFile.GetPath());
	}
	ImGui::SameLine();
	ImGui::Text("output");
	ImGui::PopID();
	if(ImGui::Button("Convert"))
	{
		char errorBuffer[64] = {0};
		if(!ConvertObj(m_allocator, Path(iPathBuffer), Path(oPathBuffer), errorBuffer))
		{
			ASSERT(false);
			Log(LogType::Warning, "Error while parsing file \"%s\": %s", iPathBuffer, errorBuffer);
		}
	}
}


REGISTER_WIDGET(obj_import)
{
	return NEW_OBJECT(allocator, ObjImportWidget)(allocator);
}


}

}