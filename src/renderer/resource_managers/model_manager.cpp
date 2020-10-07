#include "model_manager.h"

#include "core/file/blob.h"
#include "core/parsing/json.h"

#include "../renderer.h"


namespace Veng
{


ResourceType Model::RESOURCE_TYPE("model");
static ResourceType MATERIAL_TYPE("material");


ModelManager::ModelManager(Allocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(Model::RESOURCE_TYPE, allocator, fileSystem, depManager)
{}


ModelManager::~ModelManager()
{}


const char* const * ModelManager::GetSupportedFileExt() const
{
	static const char* buffer[] = { "model" };
	return buffer;
}

size_t ModelManager::GetSupportedFileExtCount() const
{
	return 1;
}


void ModelManager::SetRenderSystem(RenderSystem* renderSystem)
{
	m_renderSystem = renderSystem;
}


Resource* ModelManager::CreateResource()
{
	Resource* res = NEW_OBJECT(m_allocator, Model)(m_allocator);
	return res;
}


void ModelManager::DestroyResource(Resource* resource)
{
	Model* model = static_cast<Model*>(resource);

	for (Mesh& mesh : model->meshes)
	{
		m_renderSystem->DestroyMeshData(mesh.renderDataHandle);
		m_allocator.Deallocate(mesh.verticesData);
		m_allocator.Deallocate(mesh.indicesData);
		m_depManager->UnloadResource(MATERIAL_TYPE, static_cast<resourceHandle>(mesh.material));
	}

	DELETE_OBJECT(m_allocator, model);
}


void ModelManager::ReloadResource(Resource* resource)
{
	ASSERT2(false, "Not implemented yet");
}


void ModelManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Model* model = static_cast<Model*>(GetResource(handle));
	
	Mesh& mesh = model->meshes.PushBack();

	char errorBuffer[64] = { 0 };
	JsonValue parsedJson;
	ASSERT(JsonParseError((char*)data.GetData(), &m_allocator, &parsedJson, errorBuffer));
	ASSERT(JsonIsObject(&parsedJson));


	JsonKeyValue* verticesData = JsonObjectFind(&parsedJson, "vertices");
	if (verticesData != nullptr)
	{
		ASSERT(JsonIsObject(&verticesData->value));

		JsonKeyValue* countJson = JsonObjectFind(&verticesData->value, "count");
		ASSERT(countJson != nullptr && JsonIsInt(&countJson->value));
		size_t count = (size_t)JsonGetInt(&countJson->value);
		if (count < MAX_U32)
		{
			mesh.verticesCount = (u32)count;
		}
		else
		{
			ASSERT2(false, "Too many vertices, limit is 2^32 - 1, clamped count to this number");
			mesh.verticesCount = MAX_U32;
		}
		JsonKeyValue* positions = JsonObjectFind(&verticesData->value, "positions");
		JsonKeyValue* colors = JsonObjectFind(&verticesData->value, "colors");
		JsonKeyValue* texCoords = JsonObjectFind(&verticesData->value, "uvs");
		JsonKeyValue* normals = JsonObjectFind(&verticesData->value, "normals");
		JsonKeyValue* tangents = JsonObjectFind(&verticesData->value, "tangents");
		JsonValue* positionArr = nullptr;
		JsonValue* colorsArr = nullptr;
		JsonValue* texCoordArr = nullptr;
		JsonValue* normalsArr = nullptr;
		JsonValue* tangentsArr = nullptr;

		size_t bufferSize = 0;
		if (positions != nullptr)
		{
			ASSERT(JsonIsArray(&positions->value) && JsonArrayCount(&positions->value) == count * 3);
			positionArr = JsonArrayBegin(&positions->value);
			bufferSize += 3 * count * sizeof(float);
			mesh.varyings |= ShaderVarying_Position;
		}
		if (colors != nullptr)
		{
			ASSERT(JsonIsArray(&colors->value) && JsonArrayCount(&colors->value) == count);
			colorsArr = JsonArrayBegin(&colors->value);
			bufferSize += 4 * count * sizeof(u8);
			mesh.varyings |= ShaderVarying_Color0;
		}
		if (texCoords != nullptr)
		{
			ASSERT(JsonIsArray(&texCoords->value) && JsonArrayCount(&texCoords->value) == count * 2);
			texCoordArr = JsonArrayBegin(&texCoords->value);
			bufferSize += 2 * count * sizeof(float);
			mesh.varyings |= ShaderVarying_Texcoords0;
		}
		if(normals != nullptr)
		{
			ASSERT(JsonIsArray(&normals->value) && JsonArrayCount(&normals->value) == count * 3);
			normalsArr = JsonArrayBegin(&normals->value);
			bufferSize += 3 * count * sizeof(float);
			mesh.varyings |= ShaderVarying_Normal;
		}
		if (tangents != nullptr)
		{
			ASSERT(JsonIsArray(&tangents->value) && JsonArrayCount(&tangents->value) == count * 3);
			tangentsArr = JsonArrayBegin(&tangents->value);
			bufferSize += 3 * count * sizeof(float);
			mesh.varyings |= ShaderVarying_Tangent;
		}

		mesh.verticesData = (u8*)m_allocator.Allocate(bufferSize, alignof(float));
		u8* dataBuffer = mesh.verticesData;
		for (size_t i = 0; i < count; ++i)
		{
			if (positionArr != nullptr)
			{
				float* posBuffer = (float*)dataBuffer;
				*posBuffer = (float)JsonGetDouble(positionArr++);
				*(posBuffer + 1) = (float)JsonGetDouble(positionArr++);
				*(posBuffer + 2) = (float)JsonGetDouble(positionArr++);
				dataBuffer = (u8*)(posBuffer + 3);
			}
			if (colorsArr != nullptr)
			{
				u32* colBuffer = (u32*)dataBuffer;
				*colBuffer = (u32)JsonGetInt(colorsArr++);
				dataBuffer = (u8*)(colBuffer + 1);
			}
			if (texCoordArr != nullptr)
			{
				float* uvBuffer = (float*)dataBuffer;
				*uvBuffer = (float)JsonGetDouble(texCoordArr++);
				*(uvBuffer + 1) = (float)JsonGetDouble(texCoordArr++);
				dataBuffer = (u8*)(uvBuffer + 2);
			}
			if(normalsArr != nullptr)
			{
				float* normalBuffer = (float*)dataBuffer;
				*normalBuffer = (float)JsonGetDouble(normalsArr++);
				*(normalBuffer + 1) = (float)JsonGetDouble(normalsArr++);
				*(normalBuffer + 2) = (float)JsonGetDouble(normalsArr++);
				dataBuffer = (u8*)(normalBuffer + 3);
			}
			if (tangentsArr != nullptr)
			{
				float* tangentBuffer = (float*)dataBuffer;
				*tangentBuffer = (float)JsonGetDouble(tangentsArr++);
				*(tangentBuffer + 1) = (float)JsonGetDouble(tangentsArr++);
				*(tangentBuffer + 2) = (float)JsonGetDouble(tangentsArr++);
				dataBuffer = (u8*)(tangentBuffer + 3);
			}
		}
	}
	else
	{
		ASSERT2(false, "Missing vertex data");
		mesh.verticesData = nullptr;
	}


	JsonKeyValue* indicesData = JsonObjectFind(&parsedJson, "indices");
	if (indicesData != nullptr)
	{
		ASSERT(JsonIsObject(&indicesData->value));

		JsonKeyValue* countJson = JsonObjectFind(&indicesData->value, "count");
		ASSERT(countJson != nullptr && JsonIsInt(&countJson->value));
		size_t count = (size_t)JsonGetInt(&countJson->value);
		if (count < MAX_U32)
		{
			mesh.indicesCount = (u32)count;
		}
		else
		{
			ASSERT2(false, "Too many indices, limit is 2^32 - 1, clamped count to this number");
			mesh.indicesCount = MAX_U32;
		}
		count *= 3;//hard coded triangles here
		JsonKeyValue* indices = JsonObjectFind(&indicesData->value, "data");
		ASSERT(JsonIsArray(&indices->value) && JsonArrayCount(&indices->value) == count);
		JsonValue* indexArr = JsonArrayBegin(&indices->value);

		mesh.indicesData = (u16*)m_allocator.Allocate(count * sizeof(u16), alignof(u16));
		u16* dataBuffer = mesh.indicesData;
		for (size_t i = 0; i < count; ++i)
		{
			*dataBuffer = (u16)JsonGetInt(indexArr++);
			dataBuffer++;
		}
	}
	else
	{
		ASSERT2(false, "Missing indices");
		mesh.indicesData = nullptr;
	}


	JsonKeyValue* material = JsonObjectFind(&parsedJson, "material");
	if(material != nullptr)
	{
		ASSERT(JsonIsString(&material->value));
		const char* materialRawStr = JsonGetString(&material->value);
		Path materialPath(materialRawStr);
		mesh.material = m_depManager->LoadResource(Model::RESOURCE_TYPE, MATERIAL_TYPE, materialPath);
		Resource* material = GetResource(mesh.material);
		if(material->GetState() == Resource::State::Ready || material->GetState() == Resource::State::Failure)
		{
			FinalizeModel(model);
		}
	}

	JsonDeinit(&parsedJson);

	mesh.renderDataHandle = m_renderSystem->CreateMeshData(mesh);
}


void ModelManager::ChildResourceLoaded(resourceHandle handle, ResourceType type)
{
	for (auto& res : m_resources)
	{
		Model* model = static_cast<Model*>(res.value);
		if (model->meshes[0].material == handle)
		{
			FinalizeModel(model);
		}
	}
}


void ModelManager::FinalizeModel(Model* model)
{
	model->SetState(Resource::State::Ready);
	m_depManager->ResourceLoaded(Model::RESOURCE_TYPE, GetResourceHandle(model));
}


}