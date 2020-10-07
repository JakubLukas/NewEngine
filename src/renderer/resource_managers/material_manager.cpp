#include "material_manager.h"

#include "core/file/blob.h"
#include "core/string.h"
#include "core/logs.h"

#include "../renderer.h"
#include "core/parsing/json.h"


namespace Veng
{


ResourceType Material::RESOURCE_TYPE("material");
static ResourceType SHADER_TYPE("shader");
static ResourceType TEXTURE_TYPE("texture");


MaterialManager::MaterialManager(Allocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(Material::RESOURCE_TYPE, allocator, fileSystem, depManager)
	, m_loadingOp(m_allocator)
{

}


MaterialManager::~MaterialManager()
{

}


const char* const * MaterialManager::GetSupportedFileExt() const
{
	static const char* buffer[] = { "material" };
	return buffer;
}

size_t MaterialManager::GetSupportedFileExtCount() const
{
	return 1;
}


void MaterialManager::SetRenderSystem(RenderSystem* renderSystem)
{
	m_renderSystem = renderSystem;
}


Resource* MaterialManager::CreateResource()
{
	return NEW_OBJECT(m_allocator, Material)();
}


void MaterialManager::DestroyResource(Resource* resource)
{
	Material* material = static_cast<Material*>(resource);

	m_renderSystem->DestroyMaterialData(material->renderDataHandle);
	m_depManager->UnloadResource(SHADER_TYPE, material->shader);

	for(int i = 0; i < material->textureCount; ++i)
	{
		m_depManager->UnloadResource(TEXTURE_TYPE, material->textures[i]);
	}

	DELETE_OBJECT(m_allocator, material);
}


void MaterialManager::ReloadResource(Resource* resource)
{
	ASSERT2(false, "Not implemented yet");
}


void MaterialManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Material* material = static_cast<Material*>(GetResource(handle));
	LoadingOp op;
	op.material = handle;

	char errorBuffer[64] = { 0 };
	JsonValue parsedJson;
	ASSERT(JsonParseError((char*)data.GetData(), &m_allocator, &parsedJson, errorBuffer));
	ASSERT(JsonIsObject(&parsedJson));

	JsonKeyValue* shader = JsonObjectFind(&parsedJson, "shader");
	ASSERT(shader != nullptr && JsonIsString(&shader->value));
	Path shaderPath(JsonGetString(&shader->value));

	material->shader = m_depManager->LoadResource(Material::RESOURCE_TYPE, SHADER_TYPE, shaderPath);
	op.shader = material->shader;
	Resource* shaderRes = GetResource(material->shader);
	if(shaderRes->GetState() == Resource::State::Ready || shaderRes->GetState() == Resource::State::Failure)
		op.shaderLoaded = true;

	JsonKeyValue* textures = JsonObjectFind(&parsedJson, "textures");
	if (textures != nullptr)
	{
		ASSERT(JsonIsObject(&textures->value));
		const JsonKeyValue* val = JsonObjectCBegin(&textures->value);
		for (size_t i = 0; i < JsonObjectCount(&textures->value); ++i)
		{
			const int idx = material->textureCount++;

			const char* textureName = JsonGetString(&val->key);
			size_t textureNameLen = string::Length(textureName);
			if (textureNameLen > ShaderUniform::MAX_NAME_LENGTH)
			{
				Log(LogType::Error, "Uniform name is too long, clamped to %d chars\n", ShaderUniform::MAX_NAME_LENGTH);
				textureNameLen = ShaderUniform::MAX_NAME_LENGTH;
			}
			string::Copy(material->outputTextures[idx].name, textureName, textureNameLen);

			ASSERT(JsonIsString(&val->value));
			const Path path(JsonGetString(&val->value));
			material->textures[idx] = m_depManager->LoadResource(Material::RESOURCE_TYPE, TEXTURE_TYPE, path);
			op.textures[idx] = material->textures[idx];
			op.textureCount++;
			Resource* textureRes = GetResource(material->textures[idx]);
			if (textureRes->GetState() != Resource::State::Ready && textureRes->GetState() != Resource::State::Failure)
				op.texturesToLoad |= 1 << idx;

			val++;
		}
	}

	JsonDeinit(&parsedJson);

	material->renderDataHandle = m_renderSystem->CreateMaterialData(*material);

	if(!LoadingOpCompleted(op))
		m_loadingOp.PushBack(op);
	else
		FinalizeMaterial(material);
}


void MaterialManager::ChildResourceLoaded(resourceHandle handle, ResourceType type)
{
	for (size_t i = 0; i < m_loadingOp.GetSize(); ++i)
	{
		LoadingOp& op = m_loadingOp[i];

		if (type == SHADER_TYPE)
		{
			if (op.shader == handle)
				op.shaderLoaded = 1;
		}
		else if (type == TEXTURE_TYPE)
		{
			for (size_t i = 0; i < op.textureCount; ++i)
			{
				if (op.textures[i] == handle)
					op.texturesToLoad &= ~(1 << i);
			}
		}

		if (LoadingOpCompleted(op))
		{
			Material* material = static_cast<Material*>(GetResource(op.material));
			FinalizeMaterial(material);
			m_loadingOp.Erase(i--);
		}
	}
}


void MaterialManager::FinalizeMaterial(Material* material)
{
	material->SetState(Resource::State::Ready);
	m_depManager->ResourceLoaded(Material::RESOURCE_TYPE, GetResourceHandle(material));
}


bool MaterialManager::LoadingOpCompleted(const LoadingOp& op)
{
	return (op.shaderLoaded && op.texturesToLoad == 0);
}


}
