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

struct MaterialData
{
	bgfx::UniformHandle textureUniforms[Material::MAX_TEXTURES];
	u8 textureCount = 0;
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


struct FrameBuffer
{
	bgfx::FrameBufferHandle handle;
	u16 width;
	u16 height;
	bool screenSize;
	FramebufferTypeFlags flags;
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

	void SetMainCamera(Entity entity) override
	{
		CameraItem* cam;
		if(!m_cameras.Find(entity, cam))
		{
			Log(LogType::Warning, "Given entity doesn't have component Camera");
			ASSERT(false);
		}
		m_mainCamera = entity;
	}

	const CameraItem* GetMainCamera() const override
	{
		if(m_mainCamera == INVALID_ENTITY)
			return nullptr;

		CameraItem* cam;
		if(m_cameras.Find(m_mainCamera, cam))
			return cam;
		else
			return nullptr;
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
	Entity m_mainCamera = INVALID_ENTITY;
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
		, m_materialData(m_allocator)
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

		MaterialData invalidMaterialData;
		invalidMaterialData.textureUniforms[0] = BGFX_INVALID_HANDLE;
		m_materialData.Add(Utils::Move(invalidMaterialData));

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
		for (const auto& scene : m_scenes)
			DELETE_OBJECT(m_allocator, scene.value);

		bgfx::destroy(m_dirLightsPos);
		bgfx::destroy(m_dirLightsColor);
	}


	void Init() override
	{
		m_dirLightsPos = bgfx::createUniform("u_directionalLightPos", bgfx::UniformType::Vec4, 1);
		m_dirLightsColor = bgfx::createUniform("u_directionalLightColor", bgfx::UniformType::Vec4, 1);
	}


	void Update(float deltaTime) override
	{
		m_currentView = m_firstView - 1;//////
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


	meshRenderHandle CreateMeshData(Mesh& mesh) override
	{
		MeshData meshData;

		size_t bufferSize = 0;
		meshData.vertex_decl.begin();
		if (mesh.varyings & ShaderVarying_Position)
		{
			meshData.vertex_decl.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float);
			bufferSize += 3 * mesh.verticesCount * sizeof(float);
		}
		if (mesh.varyings & ShaderVarying_Color0)
		{
			meshData.vertex_decl.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true);
			bufferSize += 4 * mesh.verticesCount * sizeof(u8);
		}
		if (mesh.varyings & ShaderVarying_Texcoords0)
		{
			meshData.vertex_decl.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float);
			bufferSize += 2 * mesh.verticesCount * sizeof(float);
		}
		if(mesh.varyings & ShaderVarying_Normal)
		{
			meshData.vertex_decl.add(bgfx::Attrib::Normal, 3, bgfx::AttribType::Float);
			bufferSize += 3 * mesh.verticesCount * sizeof(float);
		}
		meshData.vertex_decl.end();

		meshData.vertexBufferHandle = bgfx::createVertexBuffer(bgfx::copy(mesh.verticesData, (uint32_t)bufferSize), meshData.vertex_decl);

		bufferSize = mesh.indicesCount * 3 * sizeof(u16);
		meshData.indexBufferHandle = bgfx::createIndexBuffer(bgfx::copy(mesh.indicesData, (uint32_t)(bufferSize)));

		return (meshRenderHandle)m_meshData.Add(Utils::Move(meshData));
	}

	void DestroyMeshData(meshRenderHandle handle) override
	{
		MeshData& data = m_meshData.Get((u32)handle);
		bgfx::destroy(data.vertexBufferHandle);
		bgfx::destroy(data.indexBufferHandle);

		m_meshData.Remove((u32)handle);
	}


	materialRenderHandle CreateMaterialData(Material& material) override
	{
		MaterialData matData;
		matData.textureCount = material.textureCount;

		for(int i = 0; i < matData.textureCount; ++i)
			matData.textureUniforms[i] = bgfx::createUniform(material.outputTextures[i].name, bgfx::UniformType::Int1);

		return (materialRenderHandle)m_materialData.Add(Utils::Move(matData));
	}

	void DestroyMaterialData(materialRenderHandle handle) override
	{
		MaterialData& data = m_materialData.Get((u32)handle);
		for(int i = 0; i < data.textureCount; ++i)
			bgfx::destroy(data.textureUniforms[i]);

		m_materialData.Remove((u32)handle);
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
		TextureData& data = m_textureData.Get((u32)handle);
		bgfx::destroy(data.handle);

		m_textureData.Remove((u32)handle);
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
		ShaderInternalData& data = m_shaderInternalData.Get((u32)handle);
		bgfx::destroy(data.handle);

		m_shaderInternalData.Remove((u32)handle);
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
		ShaderData& data = m_shaderData.Get((u32)handle);

		bgfx::destroy(data.handle);

		m_shaderData.Remove((u32)handle);
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
			bgfx::TextureHandle fbTextures[8];
			u8 fbTexturesSize = 0;
			if(fb.flags & FramebufferType_Color)
				fbTextures[fbTexturesSize++] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT);
			if(fb.flags & FramebufferType_Depth)
				fbTextures[fbTexturesSize++] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D32, BGFX_TEXTURE_RT);
			fb.handle = bgfx::createFrameBuffer(2, fbTextures, true);
		}
	}

	u32 GetScreenWidth() const override { return m_width; }
	u32 GetScreenHeight() const override { return m_height; }


	Engine& GetEngine() const override { return m_engine; }


	FramebufferHandle CreateFrameBuffer(int width, int height, bool screenSize, FramebufferTypeFlags flags) override
	{
		bgfx::TextureHandle fbTextures[8];
		u8 fbTexturesSize = 0;
		if(flags & FramebufferType_Color)
			fbTextures[fbTexturesSize++] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::BGRA8, BGFX_TEXTURE_RT);
		if(flags & FramebufferType_Depth)
			fbTextures[fbTexturesSize++] = bgfx::createTexture2D(width, height, false, 1, bgfx::TextureFormat::D32, BGFX_TEXTURE_RT);

		FrameBuffer fb;
		fb.handle = bgfx::createFrameBuffer(fbTexturesSize, fbTextures, true);

		fb.width = width;
		fb.height = height;
		fb.screenSize = screenSize;
		fb.flags = flags;
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
		//Camera* camera = scene->GetComponentData(componentHandle(1), ) ////////////////////////////////////////////////////////
		const RenderScene::CameraItem* cameraItem = scene->GetMainCamera();
		if (cameraItem != nullptr)
		{
			const Camera& cam = cameraItem->camera;
			const Transform& camTrans = world.GetEntityTransform(cameraItem->entity);

			if (cam.type == Camera::Type::Perspective)
			{
				proj.SetPerspective(cam.fov, cam.aspect, cam.nearPlane, cam.farPlane, bgfx::getCaps()->homogeneousDepth);
			}
			else if (cam.type == Camera::Type::Orthogonal)
			{
				float halfW = cam.screenWidth * 0.5f;
				float halfH = cam.screenHeight * 0.5f;
				proj.SetOrthogonal(-halfW, halfW, -halfH, halfH, cam.nearPlane, cam.farPlane, 0.0f, bgfx::getCaps()->homogeneousDepth);
			}
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
		const RenderScene* scene = (RenderScene*)GetScene(world.GetId());
		if(scene->GetDirectionalLightsCount() > 0)
		{
			const RenderScene::DirectionalLightItem* dirLight = scene->GetDirectionalLights();
			Transform& dirLightTrans = world.GetEntityTransform(dirLight->entity);
			Vector4 dir = Vector4(-dirLightTrans.position, 0);
			dir.Normalize();
			bgfx::setUniform(m_dirLightsPos, &dir);
			bgfx::setUniform(m_dirLightsColor, &dirLight->light);
		}

		for (size_t i = 0; i < count; ++i)
		{

			Transform& trans = world.GetEntityTransform(models[i].entity);
			Matrix44 mtx = trans.ToMatrix44();
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

						MaterialData& materialData = m_materialData.Get((u64)material->renderDataHandle);
						for(int i = 0; i < materialData.textureCount; ++i)
						{
							const Texture* texture = (Texture*)m_textureManager->GetResource(material->textures[i]);
							TextureData& texData = m_textureData.Get((u64)texture->renderDataHandle);
							bgfx::setTexture(i, materialData.textureUniforms[i], texData.handle);
						}

						uint64_t state = BGFX_STATE_DEFAULT;
						bgfx::setState(state);
						//bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS  | BGFX_STATE_CULL_CCW | BGFX_STATE_MSAA);

						const Shader* shader = (Shader*)m_shaderManager->GetResource(material->shader);
						ShaderData& shaderData = m_shaderData.Get((u64)shader->renderDataHandle);
						bgfx::submit(m_currentView, shaderData.handle);
					}
				}
			}
		}
	}


	void* GetNativeFrameBufferHandle(FramebufferHandle handle) override
	{
		return (void*)&(m_framebuffers.Get((size_t)handle).handle);
	}


private:
	ProxyAllocator m_allocator;//must be first
	Engine& m_engine;
	HashMap<worldId, RenderSceneImpl*> m_scenes;
	HandleArray<MeshData, u32> m_meshData;
	HandleArray<MaterialData, u32> m_materialData;
	HandleArray<TextureData, u32> m_textureData;
	HandleArray<ShaderInternalData, u32> m_shaderInternalData;
	HandleArray<ShaderData, u32> m_shaderData;

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

	bgfx::UniformHandle m_dirLightsPos;
	bgfx::UniformHandle m_dirLightsColor;
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