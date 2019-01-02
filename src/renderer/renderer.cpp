#include "renderer.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "core/containers/associative_array.h"
#include "core/logs.h"
#include "core/file/path.h"
#include "core/math/matrix.h"

#include "core/resource/resource_management.h"
#include "resource_managers/shader_manager.h"
#include "resource_managers/material_manager.h"
#include "resource_managers/model_manager.h"
#include "resource_managers/texture_manager.h"

#include <bgfx/bgfx.h>///////////////
#include "core/file/blob.h"//////////////////////
#include "core/file/clob.h"//////////////////////

#include "core/math/math.h"
#include "camera.h"


namespace Veng
{

static const unsigned short VIEW_ID = 1;


struct MeshData
{
	bgfx::VertexDecl vertex_decl;
	bgfx::VertexBufferHandle vertexBufferHandle;
	bgfx::IndexBufferHandle indexBufferHandle;
};

struct TextureData
{
	bgfx::TextureHandle handle;
};

struct ShaderInternalData
{
	bgfx::ShaderHandle handle;
};

struct ShaderData
{
	bgfx::ProgramHandle handle;
};


struct PosColorVertex ///////////////////this must be dynamic in future
{
	float x;
	float y;
	float z;
	u32 abgr;
	float u0;
	float v0;
};


class RenderSceneImpl : public RenderScene
{
public:
	enum : u8
	{
		COMPONENT_MODEL = 0,
		COMPONENT_CAMERA = 1,
	};

public:
	RenderSceneImpl(IAllocator& allocator, RenderSystem& renderSystem)
		: m_allocator(allocator)
		, m_renderSystem(renderSystem)
		, m_models(m_allocator)
		, m_cameras(m_allocator)
		, m_componentInfos(m_allocator)
	{
		ComponentInfo* compInfoModel;

		compInfoModel = &m_componentInfos.EmplaceBack(m_allocator);
		compInfoModel->handle = (componentHandle)COMPONENT_MODEL;
		compInfoModel->name = "model";
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::ResourceHandle, "model handle" });
		compInfoModel->dataSize = sizeof(ResourceType) + sizeof(resourceHandle);

		compInfoModel = &m_componentInfos.EmplaceBack(m_allocator);
		compInfoModel->handle = (componentHandle)COMPONENT_CAMERA;
		compInfoModel->name = "camera";
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Angle, "fov" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "near plane" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "far plane" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "screen width" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "screen height" });
		compInfoModel->dataSize = sizeof(Camera);
	}

	~RenderSceneImpl() override
	{
		for (auto& modelItem : m_models)
		{
			m_renderSystem.GetModelManager().Unload(modelItem.model);
		}
	}


	void Update(float deltaTime) override
	{}


	size_t GetComponentCount() const override { return m_componentInfos.GetSize(); }

	const ComponentInfo* GetComponents() const override { return m_componentInfos.Begin(); }

	const ComponentInfo* GetComponentInfo(componentHandle handle) const override { return &m_componentInfos[(size_t)handle]; }


	void AddComponent(componentHandle handle, Entity entity, worldId world) override
	{
		switch ((u8)handle)
		{
		case COMPONENT_MODEL:
			m_models.Insert(entity, { entity, INVALID_RESOURCE_HANDLE });
			break;
		case COMPONENT_CAMERA:
			m_cameras.Insert(entity, { entity, Camera() });
			break;
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}
	}

	void RemoveComponent(componentHandle handle, Entity entity, worldId world) override
	{
		switch ((u8)handle)
		{
		case COMPONENT_MODEL:
			m_models.Erase(entity);
			break;
		case COMPONENT_CAMERA:
			m_cameras.Erase(entity);
			break;
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}
	}

	bool HasComponent(componentHandle handle, Entity entity, worldId world) const override
	{
		switch ((u8)handle)
		{
		case COMPONENT_MODEL:
			ModelItem* model;
			return m_models.Find(entity, model);
		case COMPONENT_CAMERA:
			CameraItem* cam;
			return m_cameras.Find(entity, cam);
		default:
			ASSERT2(false, "Unrecognized componentHandle");
			return false;
		}
	}

	void GetComponentData(componentHandle handle, Entity entity, worldId world, void* buffer) const override
	{
		switch ((u8)handle)
		{
		case COMPONENT_MODEL:
		{
			ModelItem* model;
			ASSERT2(m_models.Find(entity, model), "Component not found");
			*(ResourceType*)buffer = ResourceType::Model;
			buffer = (ResourceType*)buffer + 1;
			*(resourceHandle*)buffer = model->model;
			break;
		}
		case COMPONENT_CAMERA:
		{
			CameraItem* cam;
			ASSERT2(m_cameras.Find(entity, cam), "Component not found");
			memory::Copy(buffer, &cam->camera, sizeof(Camera));
			break;
		}
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}
	}

	void SetComponentData(componentHandle handle, Entity entity, worldId world, void* data) override
	{
		switch ((u8)handle)
		{
		case COMPONENT_MODEL:
			ModelItem* model;
			ASSERT2(m_models.Find(entity, model), "Component not found");
			m_renderSystem.GetModelManager().Unload(model->model);
			model->model = *(resourceHandle*)data;
			break;
		case COMPONENT_CAMERA:
			CameraItem* cam;
			ASSERT2(m_cameras.Find(entity, cam), "Component not found");
			memory::Move(&cam->camera, data, sizeof(Camera));
			cam->camera.aspect = cam->camera.screenWidth / cam->camera.screenHeight;
			break;
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}
	}


	size_t GetModelsCount(worldId world) const override
	{
		return m_models.GetSize();
	}

	const ModelItem* GetModels(worldId world) const override
	{
		return m_models.GetValues();
	}


	size_t GetCamerasCount(worldId world) const override
	{
		return m_cameras.GetSize();
	}

	const CameraItem* GetCameras(worldId world) const override
	{
		return m_cameras.GetValues();
	}

	const CameraItem* GetDefaultCamera(worldId world) override
	{
		if (m_cameras.GetSize() == 0)
			return nullptr;
		else
			return m_cameras.GetValues();
	}

private:
	IAllocator& m_allocator;
	RenderSystem& m_renderSystem;
	Array<ComponentInfo> m_componentInfos;

	AssociativeArray<Entity, ModelItem> m_models;
	AssociativeArray<Entity, CameraItem> m_cameras;
};


//----------------------------------------------------------------------------------


class RenderSystemImpl : public RenderSystem
{
public:
	RenderSystemImpl(Engine& engine)
		: m_allocator(engine.GetAllocator())
		, m_engine(engine)
		, m_meshData(m_allocator)
		, m_textureData(m_allocator)
		, m_shaderInternalData(m_allocator)
		, m_shaderData(m_allocator)
	{
		m_allocator.SetDebugName("Renderer");

		MeshData& invalidMeshData = m_meshData.PushBack();
		invalidMeshData.vertexBufferHandle = BGFX_INVALID_HANDLE;
		invalidMeshData.indexBufferHandle = BGFX_INVALID_HANDLE;

		TextureData& invalidTextureData = m_textureData.PushBack();
		invalidTextureData.handle = BGFX_INVALID_HANDLE;

		ShaderInternalData& invalidshaderIntData = m_shaderInternalData.PushBack();
		invalidshaderIntData.handle = BGFX_INVALID_HANDLE;

		ShaderData& invalidshaderData = m_shaderData.PushBack();
		invalidshaderData.handle = BGFX_INVALID_HANDLE;

		m_shaderInternalManager = static_cast<ShaderInternalManager*>(m_engine.GetResourceManager(ResourceType::ShaderInternal));
		m_shaderInternalManager->SetRenderSystem(this);
		m_shaderManager = static_cast<ShaderManager*>(m_engine.GetResourceManager(ResourceType::Shader));
		m_shaderManager->SetRenderSystem(this);
		m_materialManager = static_cast<MaterialManager*>(m_engine.GetResourceManager(ResourceType::Material));
		m_materialManager->SetRenderSystem(this);
		m_modelManager = static_cast<ModelManager*>(m_engine.GetResourceManager(ResourceType::Model));
		m_modelManager->SetRenderSystem(this);
		m_textureManager = static_cast<TextureManager*>(m_engine.GetResourceManager(ResourceType::Texture));
		m_textureManager->SetRenderSystem(this);
	}


	~RenderSystemImpl() override
	{
		DELETE_OBJECT(m_allocator, m_scene);

		bgfx::destroy(m_uniformTextureColor);
	}


	void Init() override
	{
		m_uniformTextureColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);

		m_scene = NEW_OBJECT(m_allocator, RenderSceneImpl)(m_allocator, *this);
	}


	void Update(float deltaTime) override
	{
		worldId worldHandle = (worldId)0;
		World* world = m_engine.GetWorld(worldHandle);

		static Matrix44 view;
		static Matrix44 proj;
		const RenderScene::CameraItem* cameraItem = m_scene->GetDefaultCamera(worldHandle);
		if(cameraItem != nullptr)
		{
			const Camera& cam = cameraItem->camera;
			proj.SetPerspective(cam.fov, cam.aspect, cam.nearPlane, cam.farPlane, bgfx::getCaps()->homogeneousDepth);
			const Transform& camTrans = world->GetEntityTransform(cameraItem->entity);
			Matrix44 camRot;
			camRot.SetRotation(camTrans.rotation);
			Vector4 eye = Vector4(camTrans.position, 1);
			Vector4 at = camRot * Vector4(0, 0, -1, 0) + Vector4(camTrans.position, 1);
			view.SetLookAt(eye, at, Vector4::AXIS_Y);
		}

		bgfx::setViewTransform(VIEW_ID, &view.m11, &proj.m11);

		bgfx::touch(VIEW_ID);//dummy draw call


		// Set vertex and index buffer.
		// TODO: DUMMY test
		const RenderScene::ModelItem* modelItems = m_scene->GetModels(worldHandle);
		for(size_t i = 0; i < m_scene->GetModelsCount(worldHandle); ++i)
		{

			Transform& trans = world->GetEntityTransform(modelItems[i].entity);
			Matrix44 mtx = trans.ToMatrix44();
			mtx.Transpose();
			// Set model matrix for rendering.
			bgfx::setTransform(&mtx.m11);


			const Model* model = (Model*)m_modelManager->GetResource(modelItems[i].model);
			if(model->GetState() == Resource::State::Ready)
			{
				for(const Mesh& mesh : model->meshes)
				{
					const Material* material = (Material*)m_materialManager->GetResource(mesh.material);
					if(material->GetState() == Resource::State::Ready)
					{
						MeshData& meshData = m_meshData[(size_t)mesh.renderDataHandle];
						bgfx::setVertexBuffer(0, meshData.vertexBufferHandle);
						bgfx::setIndexBuffer(meshData.indexBufferHandle);

						const Texture* texture = (Texture*)m_textureManager->GetResource(material->textures[0]);
						TextureData& texData = m_textureData[(size_t)texture->renderDataHandle];
						bgfx::setTexture(0, m_uniformTextureColor, texData.handle);


						// Set render states.
						bgfx::setState(BGFX_STATE_DEFAULT);

						// Submit primitive for rendering to view 0.
						const Shader* shader = (Shader*)m_shaderManager->GetResource(material->shader);
						ShaderData& shaderData = m_shaderData[(size_t)shader->renderDataHandle];
						bgfx::submit(VIEW_ID, shaderData.handle);
					}
				}
			}
		}

	};


	const char* GetName() const override { return "renderer"; }


	IScene* GetScene() const override { return m_scene; }


	MaterialManager& GetMaterialManager() const override { return *m_materialManager; }
	ShaderManager& GetShaderManager() const override { return *m_shaderManager; }
	ModelManager& GetModelManager() const override { return *m_modelManager; }
	TextureManager& GetTextureManager() const override { return *m_textureManager; }


	meshRenderHandle CreateMeshData(InputBlob& data) override
	{
		InputClob dataText(data);

		MeshData& meshData = m_meshData.PushBack();

		meshData.vertex_decl
			.begin()
			.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
			.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
			.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
			.end();

		int verticesCount;
		ASSERT(dataText.Read(verticesCount));

		u32 verticesBufferSize = verticesCount * sizeof(PosColorVertex);
		PosColorVertex* vertices = (PosColorVertex*)m_allocator.Allocate(verticesBufferSize, alignof(PosColorVertex));
		for (int i = 0; i < verticesCount; ++i)
		{
			ASSERT(dataText.Read(vertices[i].x));
			dataText.Skip(1);
			ASSERT(dataText.Read(vertices[i].y));
			dataText.Skip(1);
			ASSERT(dataText.Read(vertices[i].z));
			dataText.Skip(1);
			ASSERT(dataText.Read(vertices[i].abgr));
			dataText.Skip(1);
			ASSERT(dataText.Read(vertices[i].u0));
			dataText.Skip(1);
			ASSERT(dataText.Read(vertices[i].v0));
		}

		int indicesCount;
		ASSERT(dataText.Read(indicesCount));
		indicesCount *= 3;

		u32 indicesBufferSize = indicesCount * sizeof(u16);
		u16* indices = (u16*)m_allocator.Allocate(indicesBufferSize, alignof(u16));
		for (int i = 0; i < indicesCount; ++i)
		{
			int num;
			ASSERT(dataText.Read(num));
			indices[i] = (u16)num;
		}

		meshData.vertexBufferHandle = bgfx::createVertexBuffer(bgfx::copy(vertices, verticesBufferSize), meshData.vertex_decl);
		meshData.indexBufferHandle = bgfx::createIndexBuffer(bgfx::copy(indices, indicesBufferSize));

		m_allocator.Deallocate(vertices);
		m_allocator.Deallocate(indices);

		data.SetPosition(data.GetPosition() + dataText.GetPosition());

		return meshRenderHandle(m_meshData.GetSize() - 1);
	}

	void DestroyMeshData(meshRenderHandle handle) override
	{
		MeshData& data = m_meshData[(size_t)handle];
		bgfx::destroy(data.vertexBufferHandle);
		bgfx::destroy(data.indexBufferHandle);

		//TODO: remove from m_meshData but do not corrupt handles
		data.vertexBufferHandle = BGFX_INVALID_HANDLE;
		data.indexBufferHandle = BGFX_INVALID_HANDLE;
	}


	textureRenderHandle CreateTextureData(const Texture& texture) override
	{
		TextureData texData;

		const bgfx::Memory* mem = bgfx::makeRef(texture.data, texture.width * texture.height * texture.channels);

		texData.handle = bgfx::createTexture2D(
			uint16_t(texture.width)
			, uint16_t(texture.height)
			, false //hasMipMap
			, 1
			, bgfx::TextureFormat::Enum::RGBA8
			, BGFX_TEXTURE_NONE
			, mem
		);

		if(bgfx::isValid(texData.handle))
		{
			bgfx::setName(texData.handle, texture.GetPath().GetPath());
			m_textureData.PushBack(texData);
			return textureRenderHandle(m_textureData.GetSize() - 1);
		}
		else
		{
			ASSERT2(false, "Texture was not created");
			return INVALID_TEXTURE_RENDER_HANDLE;
		}
	}

	void DestroyTextureData(textureRenderHandle handle) override
	{
		TextureData& data = m_textureData[(size_t)handle];
		bgfx::destroy(data.handle);

		//TODO: remove from m_textureData but do not corrupt handles
		data.handle = BGFX_INVALID_HANDLE;
	}


	shaderInternalRenderHandle CreateShaderInternalData(InputBlob& data) override
	{
		InputClob dataText(data);

		ShaderInternalData shaderIntData;

		const bgfx::Memory* mem = bgfx::alloc((u32)dataText.GetSize() + 1);
		memory::Copy(mem->data, dataText.GetData(), dataText.GetSize());
		mem->data[mem->size - 1] = '\0';

		shaderIntData.handle = bgfx::createShader(mem);
		if(bgfx::isValid(shaderIntData.handle))
		{
			//bgfx::setName(shaderIntData.handle, shaderInt->GetPath().path);
			m_shaderInternalData.PushBack(shaderIntData);
			return shaderInternalRenderHandle(m_shaderInternalData.GetSize() - 1);
		}
		else
		{
			ASSERT2(false, "Shader Int was not created");
			return INVALID_SHADER_INTERNAL_RENDER_HANDLE;
		}
	}

	void DestroyShaderInternalData(shaderInternalRenderHandle handle) override
	{
		ShaderInternalData& data = m_shaderInternalData[(size_t)handle];
		bgfx::destroy(data.handle);

		//TODO: remove from m_shaderInternalData but do not corrupt handles
		data.handle = BGFX_INVALID_HANDLE;
	}


	shaderRenderHandle CreateShaderData(shaderInternalRenderHandle vsHandle, shaderInternalRenderHandle fsHandle) override
	{
		ShaderData shaderData;

		ShaderInternalData vs = m_shaderInternalData[(size_t)vsHandle];
		ShaderInternalData fs = m_shaderInternalData[(size_t)fsHandle];

		shaderData.handle = bgfx::createProgram(vs.handle, fs.handle, false);

		if(bgfx::isValid(shaderData.handle))
		{
			m_shaderData.PushBack(shaderData);
			return shaderRenderHandle(m_shaderData.GetSize() - 1);
		}
		else
		{
			ASSERT2(false, "Shader was not created");
			return INVALID_SHADER_RENDER_HANDLE;
		}
	}

	void DestroyShaderData(shaderRenderHandle handle) override
	{
		ShaderData& data = m_shaderData[(size_t)handle];

		bgfx::destroy(data.handle);
	}


	void Resize(u32 width, u32 height) override
	{
		m_width = width;
		m_height = height;
	}

	u32 GetScreenWidth() const override { return m_width; }
	u32 GetScreenHeight() const override { return m_height; }


	Engine& GetEngine() const override { return m_engine; }

private:
	ProxyAllocator m_allocator;//must be first
	Engine& m_engine;
	RenderSceneImpl* m_scene;
	Array<MeshData> m_meshData;
	Array<TextureData> m_textureData;
	Array<ShaderInternalData> m_shaderInternalData;
	Array<ShaderData> m_shaderData;

	ShaderInternalManager* m_shaderInternalManager = nullptr;
	ShaderManager* m_shaderManager = nullptr;
	MaterialManager* m_materialManager = nullptr;
	ModelManager* m_modelManager = nullptr;
	TextureManager* m_textureManager = nullptr;

	/////////////////////
	u32 m_width = 0;
	u32 m_height = 0;
	/////////////////////

	bgfx::UniformHandle m_uniformTextureColor;
};


RenderSystem* RenderSystem::Create(Engine& engine)
{
	return NEW_OBJECT(engine.GetAllocator(), RenderSystemImpl)(engine);
}

void RenderSystem::Destroy(RenderSystem* system)
{
	IAllocator& allocator = system->GetEngine().GetAllocator();
	DELETE_OBJECT(allocator, system);
}

}