#include "renderer.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "core/containers/associative_array.h"
#include "core/logs.h"
#include "core/file/path.h"
#include "core/math/matrix.h"

#include "core/resource/resource_management.h"
#include "shader_manager.h"
#include "material_manager.h"
#include "model_manager.h"
#include "model.h"
#include "texture_manager.h"

#include <bgfx/bgfx.h>///////////////

#include "core/math/math.h"
#include "camera.h"


namespace Veng
{

static const unsigned short VIEW_ID = 1;

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

		compInfoModel = &m_componentInfos.EmplaceBack(m_allocator);
		compInfoModel->handle = (componentHandle)COMPONENT_CAMERA;
		compInfoModel->name = "camera";
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Angle, "fov" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "near plane" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "far plane" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "screen width" });
		compInfoModel->values.PushBack({ ComponentInfo::ValueType::Float, "screen height" });
	}

	~RenderSceneImpl() override
	{
		for (auto& modelItem : m_models)
		{
			m_renderSystem.GetModelManager().Unload(modelItem.handle);
		}
	}


	void Update(float deltaTime) override
	{

	}




	size_t GetComponentCount() const override { return m_componentInfos.GetSize(); }

	const ComponentInfo* GetComponents() const override { return m_componentInfos.Begin(); }

	const ComponentInfo* GetComponentInfo(componentHandle handle) const override { return &m_componentInfos[(size_t)handle]; }


	void AddComponent(componentHandle handle, Entity entity, worldId world) override
	{
		switch ((u8)handle)
		{
		case COMPONENT_MODEL:
			m_models.Insert(entity, { entity, INVALID_MODEL_HANDLE });
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

	void* GetComponentData(componentHandle handle, Entity entity, worldId world) const override
	{
		switch ((u8)handle)
		{
		case COMPONENT_MODEL:
			ModelItem* model;
			ASSERT2(m_models.Find(entity, model), "Component not found");
			return &model->handle;
		case COMPONENT_CAMERA:
			CameraItem* cam;
			ASSERT2(m_cameras.Find(entity, cam), "Component not found");
			return &cam->camera;
		default:
			ASSERT2(false, "Unrecognized componentHandle");
			return nullptr;
		}
	}

	void SetComponentData(componentHandle handle, Entity entity, worldId world, void* data) override
	{
		switch ((u8)handle)
		{
		case COMPONENT_MODEL:
			ModelItem* model;
			ASSERT2(m_models.Find(entity, model), "Component not found");
			model->handle = *(modelHandle*)data;
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





	/*void AddModelComponent(Entity entity, worldId world, const Path& path) override
	{
		modelHandle handle = m_renderSystem.GetModelManager().Load(path);
		m_models.Insert(entity, { entity, handle });
	}

	void RemoveModelComponent(Entity entity, worldId world) override
	{
		m_models.Erase(entity);
	}

	bool HasModelComponent(Entity entity, worldId world) const override
	{
		ModelItem* model;
		return m_models.Find(entity, model);
	}

	ModelItem* GetModelComponent(Entity entity, worldId world) const override
	{
		ModelItem* model;
		if(m_models.Find(entity, model))
			return model;
		else
			return nullptr;
	}*/

	size_t GetModelsCount(worldId world) const override
	{
		return m_models.GetSize();
	}

	const ModelItem* GetModels(worldId world) const override
	{
		return m_models.GetValues();
	}


	/*void AddCameraComponent(Entity entity, worldId world, float fovY, float near, float far) override
	{
		Camera cam;
		cam.fov = fovY;
		cam.nearPlane = near;
		cam.farPlane = far;
		cam.screenWidth = (float)m_renderSystem.GetScreenWidth();
		cam.screenHeight = (float)m_renderSystem.GetScreenHeight();
		cam.aspect = cam.screenWidth / cam.screenHeight;
		m_cameras.Insert(entity, { entity, cam });
	}

	void RemoveCameraComponent(Entity entity, worldId world) override
	{
		m_cameras.Erase(entity);
	}

	bool HasCameraComponent(Entity entity, worldId world) const override
	{
		CameraItem* cam;
		return m_cameras.Find(entity, cam);
	}

	const CameraItem* GetCameraComponent(Entity entity, worldId world) const override
	{
		CameraItem* cam;
		if(m_cameras.Find(entity, cam))
			return cam;
		else
			return nullptr;
	}

	void SetCameraFovY(Entity entity, float fovY) override
	{
		CameraItem* cam;
		if (m_cameras.Find(entity, cam))
			cam->camera.fov = fovY;
	}

	void SetCameraNearFar(Entity entity, float near, float far) override
	{
		CameraItem* cam;
		if (m_cameras.Find(entity, cam))
		{
			cam->camera.nearPlane = near;
			cam->camera.farPlane = far;
		}
	}

	void SetCameraScreenSize(Entity entity, float width, float height) override
	{
		CameraItem* cam;
		if (m_cameras.Find(entity, cam))
		{
			cam->camera.screenWidth = width;
			cam->camera.screenHeight = height;
			cam->camera.aspect = width / height;
		}
	}*/

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
	{
		m_allocator.SetDebugName("Renderer");

		m_shaderInternalManager = static_cast<ShaderInternalManager*>(m_engine.GetResourceManager(ResourceType::ShaderInternal));
		m_shaderManager = static_cast<ShaderManager*>(m_engine.GetResourceManager(ResourceType::Shader));
		m_materialManager = static_cast<MaterialManager*>(m_engine.GetResourceManager(ResourceType::Material));
		m_modelManager = static_cast<ModelManager*>(m_engine.GetResourceManager(ResourceType::Model));
		m_textureManager = static_cast<TextureManager*>(m_engine.GetResourceManager(ResourceType::Texture));
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


			const Model* model = m_modelManager->GetResource(modelItems[i].handle);
			if(model->GetState() == Resource::State::Ready)
			{
				for(const Mesh& mesh : model->meshes)
				{
					const Material* material = m_materialManager->GetResource(mesh.material);
					if(material->GetState() == Resource::State::Ready)
					{
						bgfx::setVertexBuffer(0, mesh.vertexBufferHandle);
						bgfx::setIndexBuffer(mesh.indexBufferHandle);

						const Texture* texture = m_textureManager->GetResource(material->textures[0]);
						bgfx::setTexture(0, m_uniformTextureColor, texture->handle);

						// Set render states.
						bgfx::setState(BGFX_STATE_DEFAULT);

						// Submit primitive for rendering to view 0.
						const Shader* shader = m_shaderManager->GetResource(material->shader);
						bgfx::submit(VIEW_ID, shader->program.handle);
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