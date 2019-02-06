#include "obj_import_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"

#include "core/iallocator.h"
#include "core/asserts.h"
#include "core/logs.h"
#include "core/string.h"
#include "core/parsing/json.h"
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
	char* fileBuffer = nullptr;
	{
		const FileMode fMode{
			FileMode::Access::Read,
			FileMode::ShareMode::ShareRead,
			FileMode::CreationDisposition::OpenExisting,
			FileMode::FlagNone
		};
		nativeFileHandle fh;
		if (!FS::OpenFileSync(fh, inPath, fMode))
		{
			ASSERT(false);
			string::Copy(error, "Could not open input file");
			return false;
		}

		size_t fSize = FS::GetFileSize(fh);
		fileBuffer = (char*)allocator.Allocate(fSize, alignof(char));
		size_t charsRead = 0;
		if (!FS::ReadFileSync(fh, 0, fileBuffer, fSize, charsRead))
		{
			ASSERT(false);
			string::Copy(error, "Could not read from input file");
			return false;
		}
		ASSERT2(fSize == charsRead, "Read size is not equal file size of input file");
		ASSERT2(FS::CloseFileSync(fh), "Closing of input file failed");
	}

	struct Face
	{
		u32 vIdx[3];
		u32 vtIdx[3];
		u32 vnIdx[3];
	};

	Array<Vector3> vertices(allocator);
	Array<Vector2> uvs(allocator);
	Array<Vector2> normals(allocator);
	Array<Face> faces(allocator);

	char* fileBufferPtr = fileBuffer;
	char lineBuffer[128];
	int charsUsed = 0;
	while(sscanf_s(fileBufferPtr, "%[^\t\n]\n%n", lineBuffer, 128, &charsUsed) > 0)
	{
		fileBufferPtr += charsUsed;
		if(lineBuffer[0] == 'v' && lineBuffer[1] == ' ')
		{
			Vector3& v = vertices.PushBack();
			sscanf_s(lineBuffer + 2, "%f %f %f", &v.x, &v.y, &v.z);
		}
		else if(lineBuffer[0] == 'v' && lineBuffer[1] == 't')
		{
			Vector2& v = uvs.PushBack();
			sscanf_s(lineBuffer + 3, "%f %f", &v.x, &v.y);
		}
		else if(lineBuffer[0] == 'v' && lineBuffer[1] == 'n')
		{
			Vector2& v = normals.PushBack();
			sscanf_s(lineBuffer + 3, "%f %f", &v.x, &v.y);
		}
		else if(lineBuffer[0] == 'f')
		{
			Face& f = faces.PushBack();
			sscanf_s(lineBuffer + 2, "%d/%d/%d %d/%d/%d %d/%d/%d", &f.vIdx[0], &f.vtIdx[0], &f.vnIdx[0], &f.vIdx[1], &f.vtIdx[1], &f.vnIdx[1], &f.vIdx[2], &f.vtIdx[2], &f.vnIdx[2]);
			f.vIdx[0]--;
			f.vIdx[1]--;
			f.vIdx[2]--;
			f.vtIdx[0]--;
			f.vtIdx[1]--;
			f.vtIdx[2]--;
			f.vnIdx[0]--;
			f.vnIdx[1]--;
			f.vnIdx[2]--;
		}
	}

	allocator.Deallocate(fileBuffer);

	JsonValue vPosArr;
	JsonSetArray(&vPosArr, &allocator);
	JsonValue vUvArr;
	JsonSetArray(&vUvArr, &allocator);
	JsonValue vNormArr;
	JsonSetArray(&vNormArr, &allocator);
	JsonValue indexArr;
	JsonSetArray(&indexArr, &allocator);

	JsonValue value;
	u32 indicesIdx = 0;
	for (const Face& face : faces)
	{
		const int idxs[3] = { 0, 1, 2 };// = { 0, 2, 1 }; //to fix CW vs CCW culling
		for (int i = 0; i < 3; ++i)//triangles
		{
			Vector3 pos = vertices[face.vIdx[idxs[i]]];
			JsonSetDouble(&value, pos.x);
			JsonArrayAdd(&vPosArr, &value);
			JsonSetDouble(&value, pos.y);
			JsonArrayAdd(&vPosArr, &value);
			JsonSetDouble(&value, pos.z);
			JsonArrayAdd(&vPosArr, &value);

			Vector2 uv = uvs[face.vtIdx[idxs[i]]];
			JsonSetDouble(&value, uv.x);
			JsonArrayAdd(&vUvArr, &value);
			JsonSetDouble(&value, uv.y);
			JsonArrayAdd(&vUvArr, &value);

			Vector2 norm = normals[face.vnIdx[idxs[i]]];
			JsonSetDouble(&value, norm.x);
			JsonArrayAdd(&vNormArr, &value);
			JsonSetDouble(&value, norm.y);
			JsonArrayAdd(&vNormArr, &value);

			JsonSetInt(&value, indicesIdx++);
			JsonArrayAdd(&indexArr, &value);
		}
	}

	JsonValue label;
	JsonValue vCount;
	JsonSetInt(&vCount, indicesIdx);
	JsonValue vCountStr;
	JsonSetCString(&vCountStr, "count");
	JsonValue iCount;
	JsonSetInt(&iCount, indicesIdx / 3);

	JsonValue verticesObj;
	JsonSetObject(&verticesObj, &allocator);
	JsonSetCString(&label, "count");
	JsonObjectAdd(&verticesObj, &label, &vCount);
	JsonSetCString(&label, "positions");
	JsonObjectAdd(&verticesObj, &label, &vPosArr);
	JsonSetCString(&label, "uvs");
	JsonObjectAdd(&verticesObj, &label, &vUvArr);
	JsonSetCString(&label, "normals");
	JsonObjectAdd(&verticesObj, &label, &vNormArr);

	JsonValue indicesObj;
	JsonSetObject(&indicesObj, &allocator);
	JsonSetCString(&label, "count");
	JsonObjectAdd(&indicesObj, &label, &iCount);
	JsonSetCString(&label, "data");
	JsonObjectAdd(&indicesObj, &label, &indexArr);

	JsonValue modelObj;
	JsonSetObject(&modelObj, &allocator);
	JsonSetCString(&label, "vertices");
	JsonObjectAdd(&modelObj, &label, &verticesObj);
	JsonSetCString(&label, "indices");
	JsonObjectAdd(&modelObj, &label, &indicesObj);

	JsonPrintContext prtCtx = JsonPrintContextInit(&allocator);

	unsigned int outBufferSize;
	char* outBuffer = JsonValuePrint(&prtCtx, &modelObj, true, &outBufferSize);

	{
		const FileMode fMode{
			FileMode::Access::Write,
			FileMode::ShareMode::NoShare,
			FileMode::CreationDisposition::CreateAlways,
			FileMode::FlagNone
		};
		nativeFileHandle fh;
		if (!FS::OpenFileSync(fh, outPath, fMode))
		{
			ASSERT(false);
			string::Copy(error, "Could not open output file");
			return false;
		}

		if (!FS::WriteFileSync(fh, 0, outBuffer, outBufferSize))
		{
			ASSERT(false);
			string::Copy(error, "Could not write to output file");
			return false;
		}
		ASSERT2(FS::CloseFileSync(fh), "Closing of output file failed");
	}

	JsonPrintContextDeinit(&prtCtx);

	JsonDeinit(&vPosArr);
	JsonDeinit(&vUvArr);
	JsonDeinit(&vNormArr);
	JsonDeinit(&indexArr);
	JsonDeinit(&verticesObj);
	JsonDeinit(&indicesObj);
	JsonDeinit(&modelObj);

	return true;
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