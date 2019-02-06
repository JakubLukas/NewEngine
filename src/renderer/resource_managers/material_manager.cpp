#include "material_manager.h"

#include "core/file/blob.h"

#include "../renderer.h"
#include "core/parsing/json.h"


namespace Veng
{


MaterialManager::MaterialManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
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
	m_depManager->UnloadResource(ResourceType::Shader, material->shader);

	for(int i = 0; i < material->textureCount; ++i)
	{
		m_depManager->UnloadResource(ResourceType::Texture, material->textures[i]);
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

	material->shader = m_depManager->LoadResource(ResourceType::Material, ResourceType::Shader, shaderPath);
	op.shader = material->shader;
	Resource* shaderRes = GetResource(material->shader);
	if(shaderRes->GetState() == Resource::State::Ready || shaderRes->GetState() == Resource::State::Failure)
		op.shaderLoaded = true;

	JsonKeyValue* textures = JsonObjectFind(&parsedJson, "textures");
	ASSERT(textures != nullptr && JsonIsObject(&textures->value));

	JsonKeyValue* diffuseTex = JsonObjectFind(&textures->value, "diffuse");
	if(diffuseTex != nullptr)
	{
		ASSERT(JsonIsString(&diffuseTex->value));
		Path path(JsonGetString(&diffuseTex->value));
		int idx = material->textureCount++;
		material->textures[idx] = m_depManager->LoadResource(ResourceType::Material, ResourceType::Texture, path);
		op.textures[idx] = material->textures[idx];
		op.textureCount++;
		Resource* textureRes = GetResource(material->textures[idx]);
		if(textureRes->GetState() != Resource::State::Ready && textureRes->GetState() != Resource::State::Failure)
			op.texturesToLoad |= 1 << idx;
	}

	JsonKeyValue* normalTex = JsonObjectFind(&textures->value, "normal");
	if(normalTex != nullptr)
	{
		ASSERT(JsonIsString(&normalTex->value));
		Path path(JsonGetString(&normalTex->value));
		int idx = material->textureCount++;
		material->textures[idx] = m_depManager->LoadResource(ResourceType::Material, ResourceType::Texture, path);
		op.textures[idx] = material->textures[idx];
		op.textureCount++;
		Resource* textureRes = GetResource(material->textures[idx]);
		if(textureRes->GetState() != Resource::State::Ready && textureRes->GetState() != Resource::State::Failure)
			op.texturesToLoad |= 1 << idx;
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

		if (type == ResourceType::Shader)
		{
			if (op.shader == handle)
				op.shaderLoaded = 1;
		}
		else if (type == ResourceType::Texture)
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
	m_depManager->ResourceLoaded(ResourceType::Material, GetResourceHandle(material));
}


bool MaterialManager::LoadingOpCompleted(const LoadingOp& op)
{
	return (op.shaderLoaded && op.texturesToLoad == 0);
}


}
