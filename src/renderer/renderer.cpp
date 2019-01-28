#include "renderer.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "core/containers/array.h"
#include "core/containers/handle_array.h"
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

#include "core/math/math.h"
#include "core/parsing/json.h"


namespace Veng
{


u32 HashWorldId(const worldId& world)
{
	return HashU32((u32)world);
}


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


struct FrameBuffer
{
	bgfx::FrameBufferHandle handle;
	u16 width;
	u16 height;
	bool screenSize;
};


componentHandle RenderScene::GetComponentHandle(Component comp)
{
	return (componentHandle)comp;
}


class RenderSceneImpl : public RenderScene
{
public:
	RenderSceneImpl(IAllocator& allocator, RenderSystem& renderSystem)
		: m_allocator(allocator)
		, m_renderSystem(renderSystem)
		, m_models(m_allocator)
		, m_cameras(m_allocator)
		, m_componentInfos(m_allocator)
		, m_directionalLights(m_allocator)
	{
		ComponentInfo* compInfoModel;

		compInfoModel = &m_componentInfos.EmplaceBack(m_allocator);
		compInfoModel->handle = GetComponentHandle(Component::Model);
		compInfoModel->name = "model";
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::ResourceHandle, "model handle" });
		compInfoModel->dataSize = sizeof(ResourceType) + sizeof(resourceHandle);

		compInfoModel = &m_componentInfos.EmplaceBack(m_allocator);
		compInfoModel->handle = GetComponentHandle(Component::Camera);
		compInfoModel->name = "camera";
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Angle, "fov" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "near plane" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "far plane" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "screen width" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "screen height" });
		compInfoModel->dataSize = sizeof(Camera);

		compInfoModel = &m_componentInfos.EmplaceBack(m_allocator);
		compInfoModel->handle = GetComponentHandle(Component::DirectionalLight);
		compInfoModel->name = "directional light";
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Color, "color" });
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


	void AddComponent(componentHandle handle, Entity entity) override
	{
		switch ((u8)handle)
		{
		case (u8)RenderScene::Component::Model:
		{
			m_models.Insert(entity, { entity, INVALID_RESOURCE_HANDLE });
			break;
		}
		case (u8)RenderScene::Component::Camera:
		{
			m_cameras.Insert(entity, { entity, Camera() });
			break;
		}
		case (u8)RenderScene::Component::DirectionalLight:
		{
			m_directionalLights.Insert(entity, { entity, DirectionalLight() });
			break;
		}
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}
	}

	void RemoveComponent(componentHandle handle, Entity entity) override
	{
		switch ((u8)handle)
		{
		case (u8)RenderScene::Component::Model:
		{
			m_models.Erase(entity);
			break;
		}
		case (u8)RenderScene::Component::Camera:
		{
			m_cameras.Erase(entity);
			break;
		}
		case (u8)RenderScene::Component::DirectionalLight:
		{
			m_directionalLights.Erase(entity);
			break;
		}
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}
	}

	bool HasComponent(componentHandle handle, Entity entity) const override
	{
		switch ((u8)handle)
		{
		case (u8)RenderScene::Component::Model:
		{
			ModelItem* model;
			return m_models.Find(entity, model);
		}
		case (u8)RenderScene::Component::Camera:
		{
			CameraItem* cam;
			return m_cameras.Find(entity, cam);
		}
		case (u8)RenderScene::Component::DirectionalLight:
		{
			DirectionalLightItem* light;
			return m_directionalLights.Find(entity, light);
		}
		default:
			ASSERT2(false, "Unrecognized componentHandle");
			return false;
		}
	}

	void GetComponentData(componentHandle handle, Entity entity, void* buffer) const override
	{
		switch ((u8)handle)
		{
		case (u8)RenderScene::Component::Model:
		{
			ModelItem* model;
			ASSERT2(m_models.Find(entity, model), "Component not found");
			*(ResourceType*)buffer = ResourceType::Model;
			buffer = (ResourceType*)buffer + 1;
			*(resourceHandle*)buffer = model->model;
			break;
		}
		case (u8)RenderScene::Component::Camera:
		{
			CameraItem* cam;
			ASSERT2(m_cameras.Find(entity, cam), "Component not found");
			memory::Copy(buffer, &cam->camera, sizeof(Camera));
			break;
		}
		case (u8)RenderScene::Component::DirectionalLight:
		{
			DirectionalLightItem* light;
			ASSERT2(m_directionalLights.Find(entity, light), "Component not found");
			memory::Copy(buffer, &light->light, sizeof(DirectionalLight));
			break;
		}
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}
	}

	void SetComponentData(componentHandle handle, Entity entity, void* data) override
	{
		switch ((u8)handle)
		{
		case (u8)RenderScene::Component::Model:
		{
			ModelItem* model;
			ASSERT2(m_models.Find(entity, model), "Component not found");
			if (model->model != INVALID_RESOURCE_HANDLE)
				m_renderSystem.GetModelManager().Unload(model->model);
			model->model = *(resourceHandle*)((ResourceType*)data + 1);
			break;
		}
		case (u8)RenderScene::Component::Camera:
		{
			CameraItem* cam;
			ASSERT2(m_cameras.Find(entity, cam), "Component not found");
			memory::Move(&cam->camera, data, sizeof(Camera));
			cam->camera.aspect = cam->camera.screenWidth / cam->camera.screenHeight;
			break;
		}
		case (u8)RenderScene::Component::DirectionalLight:
		{
			DirectionalLightItem* light;
			ASSERT2(m_directionalLights.Find(entity, light), "Component not found");
			memory::Move(&light->light, data, sizeof(DirectionalLight));
			break;
		}
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}
	}


	size_t GetModelsCount() const override
	{
		return m_models.GetSize();
	}

	const ModelItem* GetModels() const override
	{
		return m_models.GetValues();
	}


	size_t GetCamerasCount() const override
	{
		return m_cameras.GetSize();
	}

	const CameraItem* GetCameras() const override
	{
		return m_cameras.GetValues();
	}

	const CameraItem* GetDefaultCamera() const override
	{
		if (m_cameras.GetSize() == 0)
			return nullptr;
		else
			return m_cameras.GetValues();
	}


	size_t GetDirectionalLightsCount() const override
	{
		return m_directionalLights.GetSize();
	}
	const DirectionalLightItem* GetDirectionalLights() const override
	{
		return m_directionalLights.GetValues();
	}

private:
	IAllocator& m_allocator;
	RenderSystem& m_renderSystem;
	Array<ComponentInfo> m_componentInfos;

	AssociativeArray<Entity, ModelItem> m_models;
	AssociativeArray<Entity, CameraItem> m_cameras;
	AssociativeArray<Entity, DirectionalLightItem> m_directionalLights;
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
		, m_framebuffers(m_allocator)
		, m_screenSizeFrameBuffers(m_allocator)
		, m_scenes(m_allocator, &HashWorldId)
	{
		m_allocator.SetDebugName("Renderer");

		MeshData invalidMeshData;
		invalidMeshData.vertexBufferHandle = BGFX_INVALID_HANDLE;
		invalidMeshData.indexBufferHandle = BGFX_INVALID_HANDLE;
		m_meshData.Add(Utils::Move(invalidMeshData));

		TextureData invalidTextureData;
		invalidTextureData.handle = BGFX_INVALID_HANDLE;
		m_textureData.Add(Utils::Move(invalidTextureData));

		ShaderInternalData invalidshaderIntData;
		invalidshaderIntData.handle = BGFX_INVALID_HANDLE;
		m_shaderInternalData.Add(Utils::Move(invalidshaderIntData));

		ShaderData invalidshaderData;
		invalidshaderData.handle = BGFX_INVALID_HANDLE;
		m_shaderData.Add(Utils::Move(invalidshaderData));

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
		bgfx::destroy(m_uniformTextureColor);

		for (const auto& scene : m_scenes)
			DELETE_OBJECT(m_allocator, scene.value);
	}


	void Init() override
	{
		m_uniformTextureColor = bgfx::createUniform("s_texColor", bgfx::UniformType::Int1);
	}


	void Update(float deltaTime) override
	{
	}


	const char* GetName() const override { return "renderer"; }


	IScene* GetScene(worldId world) const override
	{
		RenderSceneImpl** scene;
		if (m_scenes.Find(world, scene))
			return *scene;
		else
			return nullptr;
	}


	void WorldAdded(worldId world) override
	{
		RenderSceneImpl* scene = NEW_OBJECT(m_allocator, RenderSceneImpl)(m_allocator, *this);
		m_scenes.Insert(world, scene);
	}

	void WorldRemoved(worldId world) override
	{
		m_scenes.Erase(world);
	}


	MaterialManager& GetMaterialManager() const override { return *m_materialManager; }
	ShaderManager& GetShaderManager() const override { return *m_shaderManager; }
	ModelManager& GetModelManager() const override { return *m_modelManager; }
	TextureManager& GetTextureManager() const override { return *m_textureManager; }


	meshRenderHandle CreateMeshData(InputBlob& data) override
	{
		char errorBuffer[64] = { 0 };
		JsonValue parsedJson;
		ASSERT(JsonParse((char*)data.GetData(), &m_allocator, &parsedJson, errorBuffer));
		ASSERT(JsonIsObject(&parsedJson));

		MeshData meshData;

		JsonKeyValue* verticesData = JsonObjectFind(&parsedJson, "vertices");
		if(verticesData != nullptr)
		{
			ASSERT(JsonIsObject(&verticesData->value));

			JsonKeyValue* countJson = JsonObjectFind(&verticesData->value, "count");
			ASSERT(countJson != nullptr && JsonIsInt(&countJson->value));
			size_t count = (size_t)JsonGetInt(&countJson->value);
			JsonKeyValue* positions = JsonObjectFind(&verticesData->value, "positions");
			JsonKeyValue* texCoords = JsonObjectFind(&verticesData->value, "uvs");
			JsonKeyValue* colors = JsonObjectFind(&verticesData->value, "colors");
			JsonValue* positionArr = nullptr;
			JsonValue* texCoordArr = nullptr;
			JsonValue* colorsArr = nullptr;

			size_t bufferSize = 0;
			meshData.vertex_decl.begin();
			if(positions != nullptr)
			{
				ASSERT(JsonIsArray(&positions->value) && JsonArrayCount(&positions->value) == count * 3);
				positionArr = JsonArrayBegin(&positions->value);
				bufferSize += 3 * count * sizeof(float);
				meshData.vertex_decl.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
			}
			if(texCoords != nullptr)
			{
				ASSERT(JsonIsArray(&texCoords->value) && JsonArrayCount(&texCoords->value) == count * 2);
				texCoordArr = JsonArrayBegin(&texCoords->value);
				bufferSize += 2 * count * sizeof(float);
				meshData.vertex_decl.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
			}
			if(colors != nullptr)
			{
				ASSERT(JsonIsArray(&colors->value) && JsonArrayCount(&colors->value) == count);
				colorsArr = JsonArrayBegin(&colors->value);
				bufferSize += 4 * count * sizeof(u8);
				meshData.vertex_decl.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
			}
			meshData.vertex_decl.end();

			u8* dataBuffer = (u8*)m_allocator.Allocate(bufferSize, alignof(float));
			for(size_t i = 0; i < count; ++i)
			{
				if(positionArr != nullptr)
				{
					float* posBuffer = (float*)dataBuffer;
					*posBuffer = (float)JsonGetDouble(positionArr++);
					*(posBuffer + 1) = (float)JsonGetDouble(positionArr++);
					*(posBuffer + 2) = (float)JsonGetDouble(positionArr++);
					dataBuffer = (u8*)(posBuffer + 3);
				}
				if(colorsArr != nullptr)
				{
					u32* colBuffer = (u32*)dataBuffer;
					*colBuffer = (u32)JsonGetInt(colorsArr++);
					dataBuffer = (u8*)(colBuffer + 1);
				}
				if(texCoordArr != nullptr)
				{
					float* uvBuffer = (float*)dataBuffer;
					*uvBuffer = (float)JsonGetDouble(texCoordArr++);
					*(uvBuffer + 1) = (float)JsonGetDouble(texCoordArr++);
					dataBuffer = (u8*)(uvBuffer + 2);
				}
			}

			meshData.vertexBufferHandle = bgfx::createVertexBuffer(bgfx::copy(dataBuffer, (uint32_t)bufferSize), meshData.vertex_decl);
			m_allocator.Deallocate(dataBuffer);
		}
		else
		{
			ASSERT2(false, "Missing vertex data");
			meshData.vertexBufferHandle = BGFX_INVALID_HANDLE;
		}

		JsonKeyValue* indicesData = JsonObjectFind(&parsedJson, "indices");
		if(indicesData != nullptr)
		{
			ASSERT(JsonIsObject(&indicesData->value));

			JsonKeyValue* countJson = JsonObjectFind(&indicesData->value, "count");
			ASSERT(countJson != nullptr && JsonIsInt(&countJson->value));
			size_t count = (size_t)JsonGetInt(&countJson->value) * 3;//hardcoded triangles here
			JsonKeyValue* indices = JsonObjectFind(&indicesData->value, "data");
			ASSERT(JsonIsArray(&indices->value) && JsonArrayCount(&indices->value) == count);
			JsonValue* indexArr = JsonArrayBegin(&indices->value);

			u16* dataBuffer = (u16*)m_allocator.Allocate(count * sizeof(u16), alignof(float));
			for(size_t i = 0; i < count; ++i)
			{
				*dataBuffer = (u16)JsonGetInt(indexArr++);
				dataBuffer++;
			}

			meshData.indexBufferHandle = bgfx::createIndexBuffer(bgfx::copy(dataBuffer, (uint32_t)(count * sizeof(u16))));
			m_allocator.Deallocate(dataBuffer);
		}
		else
		{
			ASSERT2(false, "Missing indices");
			meshData.indexBufferHandle = BGFX_INVALID_HANDLE;
		}

		JsonDeinit(&parsedJson);

		return (meshRenderHandle)m_meshData.Add(Utils::Move(meshData));
	}

	void DestroyMeshData(meshRenderHandle handle) override
	{
		MeshData& data = m_meshData.Get((u64)handle);
		bgfx::destroy(data.vertexBufferHandle);
		bgfx::destroy(data.indexBufferHandle);

		m_meshData.Remove((u64)handle);
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
			return (textureRenderHandle)m_textureData.Add(Utils::Move(texData));
		}
		else
		{
			ASSERT2(false, "Texture was not created");
			return INVALID_TEXTURE_RENDER_HANDLE;
		}
	}

	void DestroyTextureData(textureRenderHandle handle) override
	{
		TextureData& data = m_textureData.Get((u64)handle);
		bgfx::destroy(data.handle);

		m_textureData.Remove((u64)handle);
	}


	shaderInternalRenderHandle CreateShaderInternalData(InputBlob& data) override
	{
		ShaderInternalData shaderIntData;

		const bgfx::Memory* mem = bgfx::alloc((u32)data.GetSize() + 1);
		memory::Copy(mem->data, data.GetData(), data.GetSize());
		mem->data[mem->size - 1] = '\0';

		shaderIntData.handle = bgfx::createShader(mem);
		if(bgfx::isValid(shaderIntData.handle))
		{
			//bgfx::setName(shaderIntData.handle, shaderInt->GetPath().path);
			return (shaderInternalRenderHandle)m_shaderInternalData.Add(Utils::Move(shaderIntData));
		}
		else
		{
			ASSERT2(false, "Shader Int was not created");
			return INVALID_SHADER_INTERNAL_RENDER_HANDLE;
		}
	}

	void DestroyShaderInternalData(shaderInternalRenderHandle handle) override
	{
		ShaderInternalData& data = m_shaderInternalData.Get((u64)handle);
		bgfx::destroy(data.handle);

		m_shaderInternalData.Remove((u64)handle);
	}


	shaderRenderHandle CreateShaderData(shaderInternalRenderHandle vsHandle, shaderInternalRenderHandle fsHandle) override
	{
		ShaderData shaderData;

		ShaderInternalData vs = m_shaderInternalData.Get((u64)vsHandle);
		ShaderInternalData fs = m_shaderInternalData.Get((u64)fsHandle);

		shaderData.handle = bgfx::createProgram(vs.handle, fs.handle, false);

		if(bgfx::isValid(shaderData.handle))
		{
			return (shaderRenderHandle)m_shaderData.Add(Utils::Move(shaderData));
		}
		else
		{
			ASSERT2(false, "Shader was not created");
			return INVALID_SHADER_RENDER_HANDLE;
		}
	}

	void DestroyShaderData(shaderRenderHandle handle) override
	{
		ShaderData& data = m_shaderData.Get((u64)handle);

		bgfx::destroy(data.handle);
	}


	void Resize(u32 width, u32 height) override
	{
		m_width = width;
		m_height = height;

		for (FramebufferHandle handle : m_screenSizeFrameBuffers)
		{
			FrameBuffer& fb = m_framebuffers.Get((u16)handle);
			fb.width = width;
			fb.height = height;
			bgfx::destroy(fb.handle);
			fb.handle = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::Enum::RGB8);
		}
	}

	u32 GetScreenWidth() const override { return m_width; }
	u32 GetScreenHeight() const override { return m_height; }


	Engine& GetEngine() const override { return m_engine; }


	FramebufferHandle CreateFrameBuffer(int width, int height, bool screenSize) override
	{
		FrameBuffer fb;
		fb.handle = bgfx::createFrameBuffer(width, height, bgfx::TextureFormat::Enum::RGB8);
		fb.width = width;
		fb.height = height;
		fb.screenSize = screenSize;
		FramebufferHandle handle = (FramebufferHandle)m_framebuffers.Add(Utils::Move(fb));
		if (screenSize)
			m_screenSizeFrameBuffers.PushBack(handle);
		return handle;
	}

	void DestroyFramebuffer(FramebufferHandle handle) override
	{
		FrameBuffer& fb = m_framebuffers.Get((u16)handle);
		if (fb.screenSize)
			m_screenSizeFrameBuffers.Erase(handle);

		bgfx::destroy(fb.handle);
		m_framebuffers.Remove((u16)handle);
	}

	void NewView() override
	{
		m_currentView++;
	}

	void SetFramebuffer(FramebufferHandle handle) override
	{
		FrameBuffer& fb = m_framebuffers.Get((u16)handle);
		bgfx::setViewFrameBuffer(m_currentView, fb.handle);
		bgfx::setViewRect(m_currentView, 0, 0, uint16_t(fb.width), uint16_t(fb.height));
	}

	void SetCamera(World& world, Entity camera)
	{
		Matrix44 view;
		Matrix44 proj;
		RenderSceneImpl* scene = (RenderSceneImpl*)GetScene(world.GetId());
		//Camera* camera = scene->GetComponentData() ////////////////////////////////////////////////////////
		const RenderScene::CameraItem* cameraItem = scene->GetDefaultCamera();
		if (cameraItem != nullptr)
		{
			const Camera& cam = cameraItem->camera;
			proj.SetPerspective(cam.fov, cam.aspect, cam.nearPlane, cam.farPlane, bgfx::getCaps()->homogeneousDepth);
			const Transform& camTrans = world.GetEntityTransform(cameraItem->entity);
			Matrix44 camRot;
			camRot.SetRotation(camTrans.rotation);
			Vector4 eye = Vector4(camTrans.position, 1);
			Vector4 at = camRot * Vector4(0, 0, -1, 0) + Vector4(camTrans.position, 1);
			view.SetLookAt(eye, at, Vector4::AXIS_Y);
		}

		bgfx::setViewTransform(m_currentView, &view.m11, &proj.m11);
	}

	void Clear()
	{
		bgfx::setViewClear((bgfx::ViewId)m_currentView
			, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
			, 0x803030ff
			, 1.0f
			, 0
		);
	}

	void RenderModels(World& world, const RenderScene::ModelItem* models, size_t count)
	{
		for (size_t i = 0; i < count; ++i)
		{

			Transform& trans = world.GetEntityTransform(models[i].entity);
			Matrix44 mtx = trans.ToMatrix44();
			mtx.Transpose();
			// Set model matrix for rendering.
			bgfx::setTransform(&mtx.m11);


			const Model* model = (Model*)m_modelManager->GetResource(models[i].model);
			if (model->GetState() == Resource::State::Ready)
			{
				for (const Mesh& mesh : model->meshes)
				{
					const Material* material = (Material*)m_materialManager->GetResource(mesh.material);
					if (material->GetState() == Resource::State::Ready)
					{
						MeshData& meshData = m_meshData.Get((u64)mesh.renderDataHandle);
						bgfx::setVertexBuffer(0, meshData.vertexBufferHandle);
						bgfx::setIndexBuffer(meshData.indexBufferHandle);

						const Texture* texture = (Texture*)m_textureManager->GetResource(material->textureHandles[0]);
						TextureData& texData = m_textureData.Get((u64)texture->renderDataHandle);
						bgfx::setTexture(0, m_uniformTextureColor, texData.handle);


						// Set render states.
						bgfx::setState(BGFX_STATE_DEFAULT);

						// Submit primitive for rendering to view 0.
						const Shader* shader = (Shader*)m_shaderManager->GetResource(material->shader);
						ShaderData& shaderData = m_shaderData.Get((u64)shader->renderDataHandle);
						bgfx::submit(m_currentView, shaderData.handle);
					}
				}
			}
		}
	}


	void Frame() override
	{
		m_currentView = m_firstView - 1;//////
	}


	void* GetNativeFrameBufferHandle(FramebufferHandle handle) override
	{
		return (void*)&(m_framebuffers.Get((size_t)handle).handle);
	}


private:
	ProxyAllocator m_allocator;//must be first
	Engine& m_engine;
	HashMap<worldId, RenderSceneImpl*> m_scenes;
	HandleArray<MeshData, u64> m_meshData;
	HandleArray<TextureData, u64> m_textureData;
	HandleArray<ShaderInternalData, u64> m_shaderInternalData;
	HandleArray<ShaderData, u64> m_shaderData;

	ShaderInternalManager* m_shaderInternalManager = nullptr;
	ShaderManager* m_shaderManager = nullptr;
	MaterialManager* m_materialManager = nullptr;
	ModelManager* m_modelManager = nullptr;
	TextureManager* m_textureManager = nullptr;

	/////////////////////
	u32 m_width = 0;
	u32 m_height = 0;
	/////////////////////
	bgfx::ViewId m_firstView = 1;//TODO
	bgfx::ViewId m_currentView = m_firstView - 1;//TODO
	HandleArray<FrameBuffer, u16> m_framebuffers;
	Array<FramebufferHandle> m_screenSizeFrameBuffers;

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