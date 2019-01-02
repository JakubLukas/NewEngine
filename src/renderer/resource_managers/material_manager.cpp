#include "material_manager.h"

#include "core/file/blob.h"
#include "core/file/clob.h"

#include "../renderer.h"


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
	static const char* buffer[] = { "tga" };
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
	Material* material = static_cast<Material*>(GetResource(handle));
	InputClob dataText(data);

	LoadingOp& op = m_loadingOp.PushBack();
	op.material = handle;

	char shaderPath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(dataText.ReadString(shaderPath, Path::MAX_LENGTH));

	char texturePath[Path::MAX_LENGTH + 1] = { '\0' };
	ASSERT(dataText.ReadString(texturePath, Path::MAX_LENGTH));

	material->shader = m_depManager->LoadResource(ResourceType::Material, ResourceType::Shader, Path(shaderPath));
	op.shader = material->shader;

	material->textures[0] = m_depManager->LoadResource(ResourceType::Material, ResourceType::Texture, Path(texturePath));
	op.textures[0] = material->textures[0];
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
					op.texturesLoaded |= 1 << i;
			}
		}

		if (op.shaderLoaded && op.texturesLoaded > 0)
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