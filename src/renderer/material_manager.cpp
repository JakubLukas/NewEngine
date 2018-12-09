#include "material_manager.h"

#include "core/file/blob.h"
#include "core/file/clob.h"

#include "renderer.h"


namespace Veng
{


inline static materialHandle GenericToMaterialHandle(resourceHandle handle)
{
	return static_cast<materialHandle>(handle);
}

inline static resourceHandle MaterialToGenericHandle(materialHandle handle)
{
	return static_cast<resourceHandle>(handle);
}


MaterialManager::MaterialManager(IAllocator& allocator, FileSystem& fileSystem, DependencyManager* depManager)
	: ResourceManager(allocator, fileSystem, depManager)
	, m_loadingOp(m_allocator)
{

}


MaterialManager::~MaterialManager()
{

}


materialHandle MaterialManager::Load(const Path& path)
{
	return GenericToMaterialHandle(ResourceManager::Load(path));
}


void MaterialManager::Unload(materialHandle handle)
{
	ResourceManager::Unload(MaterialToGenericHandle(handle));
}


void MaterialManager::Reload(materialHandle handle)
{
	ResourceManager::Reload(MaterialToGenericHandle(handle));
}


const Material* MaterialManager::GetResource(materialHandle handle) const
{
	return static_cast<const Material*>(ResourceManager::GetResource(MaterialToGenericHandle(handle)));
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

	m_depManager->UnloadResource(ResourceType::Shader, static_cast<resourceHandle>(material->shader));

	m_depManager->UnloadResource(ResourceType::Texture, static_cast<resourceHandle>(material->textures[0]));

	DELETE_OBJECT(m_allocator, material);
}


void MaterialManager::ReloadResource(Resource* resource)
{
	ASSERT2(false, "Not implemented yet");
}


void MaterialManager::ResourceLoaded(resourceHandle handle, InputBlob& data)
{
	Material* material = static_cast<Material*>(ResourceManager::GetResource(handle));
	InputClob dataText(data);

	LoadingOp& op = m_loadingOp.PushBack();
	op.material = GenericToMaterialHandle(handle);

	char shaderPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(dataText.ReadString(shaderPath, Path::MAX_LENGTH));

	char texturePath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(dataText.ReadString(texturePath, Path::MAX_LENGTH));

	material->shader = static_cast<shaderHandle>(m_depManager->LoadResource(ResourceType::Material, ResourceType::Shader, Path(shaderPath)));
	op.shader = material->shader;

	material->textures[0] = static_cast<textureHandle>(m_depManager->LoadResource(ResourceType::Material, ResourceType::Texture, Path(texturePath)));
	op.textures[0] = material->textures[0];
}


void MaterialManager::ChildResourceLoaded(resourceHandle handle, ResourceType type)
{
	for (size_t i = 0; i < m_loadingOp.GetSize(); ++i)
	{
		LoadingOp& op = m_loadingOp[i];

		if (type == ResourceType::Shader)
		{
			if (op.shader == static_cast<shaderHandle>(handle))
				op.shaderLoaded = 1;
		}
		else if (type == ResourceType::Texture)
		{
			for (size_t i = 0; i < Material::MAX_TEXTURES; ++i)
			{
				if (op.textures[i] == static_cast<textureHandle>(handle))
					op.texturesLoaded |= 1 << i;
			}
		}

		if (op.shaderLoaded && op.texturesLoaded > 0)
		{
			Material* material = static_cast<Material*>(ResourceManager::GetResource(MaterialToGenericHandle(op.material)));
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