#include "obj_import_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"

#include "core/allocator.h"
#include "core/asserts.h"
#include "core/logs.h"
#include "core/string.h"
#include "core/parsing/json.h"
#include "core/containers/array.h"
#include "core/math/vector.h"


#include "core/os/os_utils.h"
#include "core/file/file.h"

//#include <cstdio>
#include "../external/OpenFBX/ofbx.h"


namespace Veng
{

namespace Editor
{


static bool FileOpenDialog(Path& path)
{
	os::FileDialogData data;
	data.filter = "Autodesk fbx,.fbx";
	os::GetWorkingDir(data.fileName, Path::MAX_LENGTH);
	string::Copy(data.fileName + string::Length(data.fileName), "/*.fbx");
	os::PathToNativePath(data.fileName);
	
	bool result = ShowOpenFileDialog(data);
	if (result)
		path.SetPath(data.fileName);

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
		path.SetPath(data.fileName);

	return result;
}

static u8* LoadFile(Allocator& allocator, const Path& path, size_t out_dataSize, char error[64])
{
	static const FileMode fMode{
			FileMode::Access::Read,
			FileMode::ShareMode::ShareRead,
			FileMode::CreationDisposition::OpenExisting,
			FileMode::FlagNone
	};

	nativeFileHandle fh;
	if (!FS::OpenFileSync(fh, path, fMode))
	{
		ASSERT(false);
		string::Copy(error, "Could not open input file");
		return nullptr;
	}

	size_t fSize = FS::GetFileSize(fh);
	u8* fileBuffer = (u8*)(allocator.Allocate(fSize, alignof(u8)));
	size_t charsRead = 0;
	if (!FS::ReadFileSync(fh, 0, fileBuffer, fSize, charsRead))
	{
		ASSERT(false);
		string::Copy(error, "Could not read from input file");
		return nullptr;
	}
	ASSERT2(fSize == charsRead, "Read size is not equal file size of input file");
	if (!FS::CloseFileSync(fh))
		ASSERT2(false, "Closing of input file failed");

	out_dataSize = fSize;
	return fileBuffer;
}

/*static bool ConvertObj(const ObjImportWidget::ConvertParams& params, const Path& inPath, const Path& outPath, char error[64])
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
		fileBuffer = (char*)(params.allocator->Allocate(fSize, alignof(char)));
		size_t charsRead = 0;
		if (!FS::ReadFileSync(fh, 0, fileBuffer, fSize, charsRead))
		{
			ASSERT(false);
			string::Copy(error, "Could not read from input file");
			return false;
		}
		ASSERT2(fSize == charsRead, "Read size is not equal file size of input file");
		if (!FS::CloseFileSync(fh))
			ASSERT2(false, "Closing of input file failed");
	}

	struct Face
	{
		u32 vIdx[3];
		u32 vtIdx[3];
		u32 vnIdx[3];
	};

	Array<Vector3> vertices(*params.allocator);
	Array<Vector2> uvs(*params.allocator);
	Array<Vector3> normals(*params.allocator);
	Array<Face> faces(*params.allocator);

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
			Vector3& v = normals.PushBack();
			sscanf_s(lineBuffer + 3, "%f %f %f", &v.x, &v.y, &v.z);
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

	params.allocator->Deallocate(fileBuffer);

	JsonValue vPosArr;
	JsonSetArray(&vPosArr, params.allocator);
	JsonValue vUvArr;
	JsonSetArray(&vUvArr, params.allocator);
	JsonValue vNormArr;
	JsonSetArray(&vNormArr, params.allocator);
	JsonValue indexArr;
	JsonSetArray(&indexArr, params.allocator);

	JsonValue value;
	u32 indicesIdx = 0;
	int idxs[3] = { 0, 1, 2 };
	if (params.triangleDef == ObjImportWidget::ConvertParams::TriangleDefinition::CounterClockWise)
	{
		idxs[1] = 2;
		idxs[2] = 1;
	}
	float zMult = (params.coordSystem == ObjImportWidget::ConvertParams::CoordSystemHandness::Right) ? -1.0f : 1.0f;

	for (const Face& face : faces)
	{
		for (int i = 0; i < 3; ++i)//triangles
		{
			Vector3 pos = vertices[face.vIdx[idxs[i]]];
			JsonSetDouble(&value, pos.x);
			JsonArrayAdd(&vPosArr, &value);
			JsonSetDouble(&value, pos.y);
			JsonArrayAdd(&vPosArr, &value);
			JsonSetDouble(&value, zMult * pos.z);
			JsonArrayAdd(&vPosArr, &value);

			Vector2 uv = uvs[face.vtIdx[idxs[i]]];
			JsonSetDouble(&value, uv.x);
			JsonArrayAdd(&vUvArr, &value);
			JsonSetDouble(&value, uv.y);
			JsonArrayAdd(&vUvArr, &value);

			Vector3 norm = normals[face.vnIdx[idxs[i]]];
			JsonSetDouble(&value, norm.x);
			JsonArrayAdd(&vNormArr, &value);
			JsonSetDouble(&value, norm.y);
			JsonArrayAdd(&vNormArr, &value);
			JsonSetDouble(&value, zMult * norm.z);
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
	JsonSetObject(&verticesObj, params.allocator);
	JsonSetCString(&label, "count");
	JsonObjectAdd(&verticesObj, &label, &vCount);
	JsonSetCString(&label, "positions");
	JsonObjectAdd(&verticesObj, &label, &vPosArr);
	JsonSetCString(&label, "uvs");
	JsonObjectAdd(&verticesObj, &label, &vUvArr);
	JsonSetCString(&label, "normals");
	JsonObjectAdd(&verticesObj, &label, &vNormArr);

	JsonValue indicesObj;
	JsonSetObject(&indicesObj, params.allocator);
	JsonSetCString(&label, "count");
	JsonObjectAdd(&indicesObj, &label, &iCount);
	JsonSetCString(&label, "data");
	JsonObjectAdd(&indicesObj, &label, &indexArr);

	JsonValue modelObj;
	JsonSetObject(&modelObj, params.allocator);
	JsonSetCString(&label, "vertices");
	JsonObjectAdd(&modelObj, &label, &verticesObj);
	JsonSetCString(&label, "indices");
	JsonObjectAdd(&modelObj, &label, &indicesObj);

	JsonPrintContext prtCtx = JsonPrintContextInit(params.allocator);

	uint outBufferSize;
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
}*/


ObjImportWidget::ObjImportWidget(Allocator& allocator)
	: m_allocator(allocator)
{
	m_params.allocator = &m_allocator;
}


ObjImportWidget::~ObjImportWidget()
{}


void ObjImportWidget::Init(Engine& engine, EditorInterface& editor)
{}


void ObjImportWidget::Deinit()
{}


void ObjImportWidget::Update(EventQueue& queue)
{}


void ObjImportWidget::Render(EventQueue& queue)
{
	ImGui::PushID("input");
	static char iPathBuffer[Path::BUFFER_LENGTH];
	ImGui::InputText("##input", iPathBuffer, Path::BUFFER_LENGTH);
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
		Path filePath;
		if(FileOpenDialog(filePath))
			string::Copy(iPathBuffer, filePath.GetPath());
	}



	//ImGui::SameLine();
	//ImGui::Text("input");
	ImGui::PopID();
	//
	//ImGui::PushID("output");
	//static char oPathBuffer[Path::BUFFER_LENGTH];
	//ImGui::InputText("##output", oPathBuffer, Path::BUFFER_LENGTH);
	//ImGui::SameLine();
	//if (ImGui::Button("Browse"))
	//{
	//	Path objFile;
	//	if (FileSaveDialog(objFile))
	//		string::Copy(oPathBuffer, objFile.GetPath());
	//}
	//ImGui::SameLine();
	//ImGui::Text("output");
	//ImGui::PopID();
	//
	//static const char* coordSystems[] = { "Right hand", "Left hand" };
	//if (ImGui::BeginCombo("coordinate system", coordSystems[(int)m_params.coordSystem], ImGuiComboFlags_None))
	//{
	//	for (int i = 0; i < sizeof(coordSystems) / sizeof(*coordSystems); ++i)
	//	{
	//		bool isSelected = m_params.coordSystem == ConvertParams::CoordSystemHandness(i);
	//		if (ImGui::Selectable(coordSystems[i], isSelected))
	//			m_params.coordSystem = ConvertParams::CoordSystemHandness(i);
	//		if (isSelected)
	//			ImGui::SetItemDefaultFocus();
	//	}
	//	ImGui::EndCombo();
	//}
	//
	//static const char* triangleDefs[] = { "Clock wise", "Counter clock wise" };
	//if (ImGui::BeginCombo("triangle definition", triangleDefs[(int)m_params.triangleDef], ImGuiComboFlags_None))
	//{
	//	for (int i = 0; i < sizeof(triangleDefs) / sizeof(*triangleDefs); ++i)
	//	{
	//		bool isSelected = m_params.triangleDef == ConvertParams::TriangleDefinition(i);
	//		if (ImGui::Selectable(triangleDefs[i], isSelected))
	//			m_params.triangleDef = ConvertParams::TriangleDefinition(i);
	//		if (isSelected)
	//			ImGui::SetItemDefaultFocus();
	//	}
	//	ImGui::EndCombo();
	//}
	//
	if(ImGui::Button("Convert"))
	{
		size_t fileBufferSize = 0;
		char errorBuffer[64] = {0};
		u8* fileBuffer = LoadFile(m_allocator, Path(iPathBuffer), fileBufferSize, errorBuffer);
		if (fileBuffer == nullptr)
		{
			ASSERT(false);
			Log(LogType::Warning, "Error while parsing file \"%s\": %s", iPathBuffer, errorBuffer);
		}
		else
		{
			m_scene = ofbx::load((ofbx::u8*)fileBuffer, (int)fileBufferSize, (ofbx::u64)ofbx::LoadFlags::TRIANGULATE);
			if (m_scene == nullptr)
			{
				ASSERT(false);
				Log(LogType::Error, "Error while ofbx::load \"%s\"", ofbx::getError());

			}

			const ofbx::Object* root = m_scene->getRoot();
			if (root)
			{
				ofbx::Object::Type objType = root->getType();
			}
		}
	}
}


REGISTER_WIDGET(obj_import)
{
	return NEW_OBJECT(allocator, ObjImportWidget)(allocator);
}


}

}
