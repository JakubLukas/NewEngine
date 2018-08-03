#include "irenderer.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "core/associative_array.h"
#include "core/logs.h"
#include "core/file/path.h"

#include "core/resource/resource_management.h"
#include "shader_manager.h"
#include "material_manager.h"
#include "model_manager.h"
#include "model.h"

#include <bgfx/bgfx.h>///////////////

#include "core/math.h"
#include "camera.h"


namespace Veng
{

static const unsigned short VIEW_ID = 1;

class RenderSceneImpl : public RenderScene
{
public:
	RenderSceneImpl(IAllocator& allocator, RenderSystem& renderSystem)
		: m_allocator(allocator)
		, m_renderSystem(renderSystem)
		, m_models(m_allocator)
		, m_cameras(m_allocator)
	{

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


	void AddModelComponent(Entity entity, worldId world, const Path& path) override
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

	size_t GetModelsCount(worldId world) const override
	{
		return m_models.GetSize();
	}

	const ModelItem* GetModels(worldId world) const override
	{
		return m_models.getValues();
	}


	void AddCameraComponent(Entity entity, worldId world, float fovY, float near, float far) override
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

	size_t GetCamerasCount(worldId world) const override
	{
		return m_cameras.GetSize();
	}

	const CameraItem* GetCameras(worldId world) const override
	{
		return m_cameras.getValues();
	}

	const CameraItem* GetDefaultCamera(worldId world) override
	{
		if (m_cameras.GetSize() == 0)
			return nullptr;
		else
			return m_cameras.getValues();
	}

private:
	IAllocator& m_allocator;
	RenderSystem& m_renderSystem;

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
		ResourceManagement* resourceManagement = m_engine.GetResourceManagement();

		m_shaderInternalManager = NEW_OBJECT(m_allocator, ShaderInternalManager)(m_allocator, *m_engine.GetFileSystem(), resourceManagement);
		m_shaderManager = NEW_OBJECT(m_allocator, ShaderManager)(m_allocator, *m_engine.GetFileSystem(), resourceManagement);
		m_materialManager = NEW_OBJECT(m_allocator, MaterialManager)(m_allocator, *m_engine.GetFileSystem(), resourceManagement);
		m_modelManager = NEW_OBJECT(m_allocator, ModelManager)(m_allocator, *m_engine.GetFileSystem(), resourceManagement);

		resourceManagement->RegisterManager(ResourceType::ShaderInternal, m_shaderInternalManager);
		resourceManagement->RegisterManager(ResourceType::Shader, m_shaderManager);
		resourceManagement->RegisterManager(ResourceType::Material, m_materialManager);
		resourceManagement->RegisterManager(ResourceType::Model, m_modelManager);
	}


	~RenderSystemImpl() override
	{
		DELETE_OBJECT(m_allocator, m_scene);

		DELETE_OBJECT(m_allocator, m_modelManager);
		DELETE_OBJECT(m_allocator, m_materialManager);
		DELETE_OBJECT(m_allocator, m_shaderManager);
		DELETE_OBJECT(m_allocator, m_shaderInternalManager);
	}


	void Init() override
	{
		m_scene = NEW_OBJECT(m_allocator, RenderSceneImpl)(m_allocator, *this);
	}


	void Update(float deltaTime) override
	{
		static float time = 0;
		time += deltaTime * 0.001f;
		static const Vector4 at = Vector4(0.0f, 0.0f, 0.0f, 1.0f);
		static const Vector4 eye = Vector4(0.0f, 0.0f, 35.0f, 1.0f);
		static const Vector4 up = Vector4(0.0f, 1.0f, 0.0f, 0.0f);

		static Matrix44 view;
		view.SetLookAt(eye, at, up);

		static Matrix44 proj;
		const RenderScene::CameraItem* cameraItem = m_scene->GetDefaultCamera(0);
		if(cameraItem != nullptr)
		{
			const Camera& cam = cameraItem->camera;
			proj.SetPerspective(cam.fov, cam.aspect, cam.nearPlane, cam.farPlane, bgfx::getCaps()->homogeneousDepth);
		}

		//float projj[16];
		//bx::mtxProj(projj, 60.0f, float(m_width) / float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);

		bgfx::setViewTransform(VIEW_ID, &view.m11, &proj.m11/*projj*/);

		bgfx::touch(VIEW_ID);//dummy draw call

		World* world = m_engine.GetWorld(0);


		// Set vertex and index buffer.
		// TODO: DUMMY test
		const RenderScene::ModelItem* modelItems = m_scene->GetModels(0);
		for(size_t i = 0; i < m_scene->GetModelsCount(0); ++i)
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


	IScene* GetScene() const override
	{
		return m_scene;
	}


	MaterialManager& GetMaterialManager() const override { return *m_materialManager; }
	ShaderManager& GetShaderManager() const override { return *m_shaderManager; }
	ModelManager& GetModelManager() const override { return *m_modelManager; }


	void Resize(u32 width, u32 height) override
	{
		m_width = width;
		m_height = height;
	}

	u32 GetScreenWidth() const override { return m_width; }
	u32 GetScreenHeight() const override { return m_height; }


	Engine& GetEngine() const override { return m_engine; }

private:
	HeapAllocator m_allocator;//must be first
	Engine& m_engine;
	RenderSceneImpl* m_scene;

	ShaderInternalManager* m_shaderInternalManager;
	ShaderManager* m_shaderManager;
	MaterialManager* m_materialManager;
	ModelManager* m_modelManager;

	/////////////////////
	u32 m_width = 0;
	u32 m_height = 0;
	/////////////////////
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