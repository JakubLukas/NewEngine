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

	m_depManager->UnloadResource(ResourceType::Shader, material->shader);

	for(int i = 0; i < Material::MAX_TEXTURES; ++i)
	{
		if(material->textureHandles[i] != INVALID_RESOURCE_HANDLE)
			m_depManager->UnloadResource(ResourceType::Texture, material->textureHandles[i]);
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
	LoadingOp& op = m_loadingOp.PushBack();
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

	JsonKeyValue* textures = JsonObjectFind(&parsedJson, "textures");
	ASSERT(textures != nullptr && JsonIsObject(&textures->value));

	JsonKeyValue* diffuseTex = JsonObjectFind(&textures->value, "diffuse");
	if(diffuseTex != nullptr)
	{
		ASSERT(JsonIsString(&diffuseTex->value));
		Path path(JsonGetString(&diffuseTex->value));
		material->textures |= ST_DIFF_TEXTURE_BIT;
		material->textureHandles[0] = m_depManager->LoadResource(ResourceType::Material, ResourceType::Texture, path);
		op.textures[0] = material->textureHandles[0];
		op.texturesToLoad |= ST_DIFF_TEXTURE_BIT;
	}

	JsonKeyValue* normalTex = JsonObjectFind(&textures->value, "normal");
	if(normalTex != nullptr)
	{
		ASSERT(JsonIsString(&normalTex->value));
		Path path(JsonGetString(&normalTex->value));
		material->textures |= ST_NORM_TEXTURE_BIT;
		material->textureHandles[1] = m_depManager->LoadResource(ResourceType::Material, ResourceType::Texture, path);
		op.textures[1] = material->textureHandles[1];
		op.texturesToLoad |= ST_NORM_TEXTURE_BIT;
	}

	JsonDeinit(&parsedJson);
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
			for (size_t i = 0; i < Material::MAX_TEXTURES; ++i)
			{
				if (op.textures[i] == handle)
					op.texturesToLoad ^= 1 << i;
			}
		}

		if (op.shaderLoaded && op.texturesToLoad == 0)
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


}