#include "renderer.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "core/containers/array.h"
#include "core/containers/handle_array.h"
#include "core/containers/associative_array.h"
#include "core/hashes.h"
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
#include "core/math/math_ext.h"
#include "core/parsing/json.h"
#include "core/string.h"

#include "editor/editor_interface.h"


namespace Veng
{


static u32 HashWorldId(const worldId& world)
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


struct DebugObject
{
	Transform transform;
	Mesh mesh;
	float lifetime;
};


class RenderSceneImpl : public RenderScene
{
public:
	friend class RenderSceneEditorImpl;

	struct ModelItem
	{
		Entity entity;
		resourceHandle model;
	};

public:
	RenderSceneImpl(Allocator& allocator, RenderSystem& renderSystem)
		: m_allocator(allocator)
		, m_renderSystem(renderSystem)
		, m_models(m_allocator)
		, m_cameras(m_allocator)
		, m_directionalLights(m_allocator)
	{
	}

	~RenderSceneImpl() override
	{
		for (auto& modelItem : m_models)
		{
			m_renderSystem.GetModelManager().Unload(modelItem.model);
		}
	}

	void Serialize(OutputBlob& serializer) const override
	{
		SerializeModels(serializer);
		SerializeCameras(serializer);
		SerializeDirLights(serializer);
		serializer.Write(m_activeCamera);
	}

	virtual void Deserialize(InputBlob& serializer)
	{
		DeserializeModels(serializer);
		DeserializeCameras(serializer);
		DeserializeDirLights(serializer);
		serializer.Read(m_activeCamera);
	}

	void Clear() override
	{}

	void Update(float deltaTime) override
	{}


	void AddModel(Entity entity) override
	{
		m_models.Insert(entity, { entity, INVALID_RESOURCE_HANDLE });
	}

	void RemoveModel(Entity entity) override
	{
		m_models.Erase(entity);
	}

	bool HasModel(Entity entity) const override
	{
		ModelItem* model;
		return m_models.Find(entity, model);
	}

	ModelData GetModelData(Entity entity) const override
	{
		ModelItem* modelItem = nullptr;
		if (m_models.Find(entity, modelItem))
		{
			Resource* res = m_renderSystem.GetModelManager().GetResource(modelItem->model);
			return ModelData{ res->GetPath() };
		}
		else
		{
			Log(LogType::Warning, "Model component for entity %d was not found", (u64)entity);
			return ModelData();
		}
	}

	void SetModelData(Entity entity, const ModelData& data) override
	{
		ModelItem* modelItem;
		if (m_models.Find(entity, modelItem))
		{
			modelItem->model = m_renderSystem.GetModelManager().Load(data.path);
		}
		else
		{
			Log(LogType::Warning, "Model component for entity %d was not found", (u64)entity);
		}
	}

	void SerializeModels(OutputBlob& serializer) const
	{
		serializer.Write((u64)m_models.GetSize());
		for (const ModelItem& model : m_models)
		{
			serializer.Write(model.entity);
			Resource* res = m_renderSystem.GetModelManager().GetResource(model.model);
			serializer.Write(res->GetPath());
		}
	}

	void DeserializeModels(InputBlob& serializer)
	{
		u64 count;
		serializer.Read(count);

		for (u64 i = 0; i < count; ++i)
		{
			Entity entity;
			serializer.Read(entity);
			ModelItem* item = m_models.Insert(entity, ModelItem{ entity });
			Path resPath;
			serializer.Read(resPath);
			item->model = m_renderSystem.GetModelManager().Load(resPath);
		}
	}
	
	const ModelItem* GetModels(size_t& out_count) const
	{
		out_count = m_models.GetSize();
		return m_models.GetValues();
	}


	void AddCamera(Entity entity) override
	{
		m_cameras.Insert(entity, { entity, Camera() });
	}

	void RemoveCamera(Entity entity) override
	{
		m_cameras.Erase(entity);
	}

	bool HasCamera(Entity entity) const override
	{
		CameraItem* cam;
		return m_cameras.Find(entity, cam);
	}

	const CameraItem* GetCameraData(Entity entity) const override
	{
		CameraItem* cameraItem = nullptr;
		if (!m_cameras.Find(entity, cameraItem))
		{
			Log(LogType::Warning, "Camera component for entity %d was not found", (u64)entity);
		}
		return cameraItem;
	}

	void SetCameraData(Entity entity, const CameraItem& data) override
	{
		CameraItem* cameraItem;
		if (m_cameras.Find(entity, cameraItem))
		{
			cameraItem->camera = data.camera;
		}
		else
		{
			Log(LogType::Warning, "Camera component for entity %d was not found", (u64)entity);
		}
	}

	void SerializeCameras(OutputBlob& serializer) const
	{
		serializer.Write((u64)m_cameras.GetSize());
		for (const CameraItem& camera : m_cameras)
		{
			serializer.Write(camera.entity);
			serializer.Write(camera.camera);
		}
	}

	void DeserializeCameras(InputBlob& serializer)
	{
		u64 count;
		serializer.Read(count);

		for (u64 i = 0; i < count; ++i)
		{
			Entity entity;
			serializer.Read(entity);
			CameraItem* item = m_cameras.Insert(entity, { entity });
			serializer.Read(item->camera);
		}
	}

	size_t GetCamerasCount() const override
	{
		return m_cameras.GetSize();
	}

	const CameraItem* GetCameras() const override
	{
		return m_cameras.GetValues();
	}

	void SetActiveCamera(Entity entity) override
	{
		CameraItem* cam;
		if(!m_cameras.Find(entity, cam))
		{
			Log(LogType::Warning, "Given entity doesn't have component Camera");
			ASSERT(false);
		}
		m_activeCamera = entity;
	}

	const CameraItem* GetActiveCamera() const override
	{
		if(m_activeCamera == INVALID_ENTITY)
			return nullptr;

		CameraItem* cam;
		if(m_cameras.Find(m_activeCamera, cam))
			return cam;
		else
			return nullptr;
	}


	void AddDirectionalLight(Entity entity) override
	{
		m_directionalLights.Insert(entity, { entity, DirectionalLight() });
	}

	void RemoveDirectionalLight(Entity entity) override
	{
		m_directionalLights.Erase(entity);
	}

	bool HasDirectionalLight(Entity entity) const override
	{
		DirectionalLightItem* light;
		return m_directionalLights.Find(entity, light);
	}

	const DirectionalLightItem* GetDirectionalLightData(Entity entity) const override
	{
		DirectionalLightItem* lightItem = nullptr;
		if (m_directionalLights.Find(entity, lightItem))
		{
			Log(LogType::Warning, "Directional light component for entity %d was not found", (u64)entity);
		}
		return lightItem;
	}

	void SetDirectionalLightData(Entity entity, const DirectionalLightItem& data) override
	{
		DirectionalLightItem* lightItem;
		if (m_directionalLights.Find(entity, lightItem))
		{
			lightItem->light = data.light;
		}
		else
		{
			Log(LogType::Warning, "Directional light component for entity %d was not found", (u64)entity);
		}
	}

	void SerializeDirLights(OutputBlob& serializer) const
	{
		serializer.Write((u64)m_directionalLights.GetSize());
		for (const DirectionalLightItem& light : m_directionalLights)
		{
			serializer.Write(light.entity);
			serializer.Write(light.light);
		}
	}

	void DeserializeDirLights(InputBlob& serializer)
	{
		u64 count;
		serializer.Read(count);

		for (u64 i = 0; i < count; ++i)
		{
			Entity entity;
			serializer.Read(entity);
			DirectionalLightItem* item = m_directionalLights.Insert(entity, { entity });
			serializer.Read(item->light);
		}
	}

	size_t GetDirectionalLightsCount() const override
	{
		return m_directionalLights.GetSize();
	}
	const DirectionalLightItem* GetDirectionalLights() const override
	{
		return m_directionalLights.GetValues();
	}


	virtual bool RaycastModels(const Ray& ray, RayHit* out_hitModel = nullptr) const override
	{
		World* world = m_renderSystem.GetEngine().GetWorld(worldId(0));

		const ModelManager& manager = m_renderSystem.GetModelManager();
		for (const ModelItem& item : m_models)
		{
			const Model* model = (Model*)manager.GetResource(item.model);
			const Transform& transform = world->GetEntityTransform(item.entity);
			Matrix44 modelMatrix = transform.ToMatrix44();
			for (size_t meshIdx = 0, meshCount = model->meshes.GetSize(); meshIdx < meshCount; ++meshIdx)
			{
				const Mesh& mesh = model->meshes[meshIdx];

				u8* vertices = mesh.verticesData;
				size_t vertexOffset = 3 * sizeof(float);
				if (mesh.varyings & ShaderVarying_Color0)
					vertexOffset += sizeof(u32);
				if (mesh.varyings & ShaderVarying_Texcoords0)
					vertexOffset += 2 * sizeof(float);
				if (mesh.varyings & ShaderVarying_Texcoords1)
					vertexOffset += 2 * sizeof(float);
				if (mesh.varyings & ShaderVarying_Normal)
					vertexOffset += 3 * sizeof(float);
				if (mesh.varyings & ShaderVarying_Tangent)
					vertexOffset += 3 * sizeof(float);
				u16* indices = mesh.indicesData;
				for (u32 i = 0, indexCount = mesh.indicesCount; i < indexCount; ++i)
				{
					u16 i1 = *(indices++);
					u16 i2 = *(indices++);
					u16 i3 = *(indices++);

					float* vertexPos = (float*)(vertices + (i1 * vertexOffset));
					Vector3 v1 = Matrix44::Multiply(modelMatrix, Vector4(*vertexPos, *(vertexPos + 1), *(vertexPos + 2), 1.0f)).GetXYZ();
					vertexPos = (float*)(vertices + (i2 * vertexOffset));
					Vector3 v2 = Matrix44::Multiply(modelMatrix, Vector4(*vertexPos, *(vertexPos + 1), *(vertexPos + 2), 1.0f)).GetXYZ();
					vertexPos = (float*)(vertices + (i3 * vertexOffset));
					Vector3 v3 = Matrix44::Multiply(modelMatrix, Vector4(*vertexPos, *(vertexPos + 1), *(vertexPos + 2), 1.0f)).GetXYZ();

					Vector3 d1 = v3 - v1;
					d1.Normalize();
					Vector3 d2 = v2 - v3;
					d2.Normalize();

					Vector3 normal = Vector3::Cross(d1, d2);
					normal.Normalize();

					if (Vector3::Dot(normal, -ray.direction) < 0.0f)
						continue; //backface

					const float epsilon = 0.001f;
					if (absf(Vector3::Dot(normal, -ray.direction)) < epsilon)
						continue;//ray is parallel

					//ray triangle intersection
					float D = Vector3::Dot(normal, v1);
					float t = (-Vector3::Dot(normal, ray.origin) + D) / Vector3::Dot(normal, ray.direction);

					if (t < 0)
						continue;//triangle is behind

					Vector3 intersection = ray.origin + t * ray.direction;

					Vector3 C; // vector perpendicular to triangle's plane

					Vector3 edge1 = v2 - v1;
					Vector3 vp1 = intersection - v1;
					C = Vector3::Cross(vp1, edge1);
					if (Vector3::Dot(normal, C) < 0.0f)
						continue;//point is outside of line

					Vector3 edge2 = v3 - v2;
					Vector3 vp2 = intersection - v2;
					C = Vector3::Cross(vp2, edge2);
					if (Vector3::Dot(normal, C) < 0.0f)
						continue;//point is outside of line

					Vector3 edge3 = v1 - v3;
					Vector3 vp3 = intersection - v3;
					C = Vector3::Cross(vp3, edge3);
					if (Vector3::Dot(normal, C) < 0.0f)
						continue;//point is outside of line

					if (out_hitModel)
						*out_hitModel = { item.entity, intersection, normal };/////////////TODO: find closest, not first

					return true;
				}
			}
		}
		return false;
	}

private:
	Allocator& m_allocator;
	RenderSystem& m_renderSystem;

	AssociativeArray<Entity, ModelItem> m_models;
	AssociativeArray<Entity, CameraItem> m_cameras;
	Entity m_activeCamera = INVALID_ENTITY;
	AssociativeArray<Entity, DirectionalLightItem> m_directionalLights;
};

//----------------------------------------------------------------------------------


class RenderSceneEditorImpl : public RenderSceneEditor
{
public:
	RenderSceneEditorImpl(RenderSystem& system) : m_system(system) {}
	~RenderSceneEditorImpl() override {}

	void EditComponent(EditorInterface& editor, const ComponentBase& component, worldId world, Entity entity) override
	{
		RenderSceneImpl* scene = static_cast<RenderSceneImpl*>(m_system.GetScene(world));
		ASSERT(scene);

		switch (component.type)
		{
		case crc32_string("Model"):
		{
			RenderSceneImpl::ModelItem* modelItem;
			if (scene->m_models.Find(entity, modelItem))
			{
				editor.EditResource("model", Model::RESOURCE_TYPE, modelItem->model, EditorInterface::EditFlag_None);
			}
			else {
				ASSERT2(false, "Entity does not have model component");
			}
		}
		break;
		case crc32_string("Camera"):
		{
			RenderScene::CameraItem* cameraItem;
			if (scene->m_cameras.Find(entity, cameraItem))
			{
				Camera& camera = cameraItem->camera;
				const char* cameraTypeNames[] = { "Orthogonal" , "Perspective" };
				u32 typeIdx = (u32)camera.type;
				if (editor.EditEnum("type", typeIdx, cameraTypeNames, sizeof(cameraTypeNames) / sizeof(cameraTypeNames[0])))
					camera.type = (Camera::Type)typeIdx;
				editor.EditFloat("screen width", camera.screenWidth, EditorInterface::EditFlag_ReadOnly);
				editor.EditFloat("screen height", camera.screenHeight, EditorInterface::EditFlag_ReadOnly);
				editor.EditFloat("near plane", camera.nearPlane);
				editor.EditFloat("far plane", camera.farPlane);
				if (camera.type == Camera::Type::Perspective)
				{
					float fovDeg = toDeg(camera.fov);
					if (editor.EditFloat("fov", fovDeg))
						camera.fov = toRad(fovDeg);

				}
			}
			else {
				ASSERT2(false, "Entity does not have camera component");
			}
		}
		break;
		case crc32_string("Directional light"):
		{
			RenderScene::DirectionalLightItem* lightItem;
			if (scene->m_directionalLights.Find(entity, lightItem))
			{
				DirectionalLight& light = lightItem->light;
				editor.EditColor("diffuse color", light.diffuseColor, EditorInterface::EditFlag_None);
				editor.EditColor("specular color", light.specularColor, EditorInterface::EditFlag_None);
			}
		}
		break;
		}
	}

private:
	RenderSystem& m_system;

};

//----------------------------------------------------------------------------------


class RenderSystemImpl : public RenderSystem
{
	enum class Version : u32
	{
		First = 0,
		Latest
	};

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
		, m_debugObjects(m_allocator)
		, m_sceneEditor(*this)
		, m_shaderInternalManager(m_allocator, *engine.GetFileSystem(), engine.GetResourceManagement())
		, m_shaderManager(m_allocator, *engine.GetFileSystem(), engine.GetResourceManagement())
		, m_materialManager(m_allocator, *engine.GetFileSystem(), engine.GetResourceManagement())
		, m_modelManager(m_allocator, *engine.GetFileSystem(), engine.GetResourceManagement())
		, m_textureManager(m_allocator, *engine.GetFileSystem(), engine.GetResourceManagement())
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

		m_shaderInternalManager.SetRenderSystem(this);
		engine.AddResourceManager(m_shaderInternalManager);
		m_shaderManager.SetRenderSystem(this);
		engine.AddResourceManager(m_shaderManager);
		m_materialManager.SetRenderSystem(this);
		engine.AddResourceManager(m_materialManager);
		m_modelManager.SetRenderSystem(this);
		engine.AddResourceManager(m_modelManager);
		m_textureManager.SetRenderSystem(this);
		engine.AddResourceManager(m_textureManager);
	}


	~RenderSystemImpl() override
	{
		for (DebugObject& dObject : m_debugObjects)
		{
			DestroyMeshData(dObject.mesh.renderDataHandle);
			m_allocator.Deallocate(dObject.mesh.verticesData);
			m_allocator.Deallocate(dObject.mesh.indicesData);
			m_engine.GetResourceManagement()->UnloadResource(Material::RESOURCE_TYPE, dObject.mesh.material);
		}

		for (const auto& scene : m_scenes)
			DELETE_OBJECT(m_allocator, scene.value);

		bgfx::destroy(m_cameraPos);
		bgfx::destroy(m_dirLightsDirs);
		bgfx::destroy(m_dirLightsColor);
	}


	u32 GetVersion() const override { return (u32)Version::Latest; }

	void Serialize(OutputBlob& serializer) const override
	{
		serializer.Write((u32)m_scenes.GetSize());
		for (const auto& item : m_scenes)
		{
			serializer.Write(item.key);
			item.value->Serialize(serializer);
		}
	}

	void Deserialize(InputBlob& serializer) override
	{
		for (const auto& scene : m_scenes)
			DELETE_OBJECT(m_allocator, scene.value);
		m_scenes.Clear();

		u32 count;
		serializer.Read(count);
		for (u32 i = 0; i < count; ++i)
		{
			worldId world;
			serializer.Read(world);
			RenderSceneImpl* scene = NEW_OBJECT(m_allocator, RenderSceneImpl)(m_allocator, *this);
			scene->Deserialize(serializer);
			m_scenes.Insert(world, scene);
		}
	}


	void Init() override
	{
		m_cameraPos = bgfx::createUniform("u_cameraPos", bgfx::UniformType::Vec4, 1);
		m_dirLightsDirs = bgfx::createUniform("u_directionalLightDir", bgfx::UniformType::Vec4, 1);
		m_dirLightsColor = bgfx::createUniform("u_directionalLightColor", bgfx::UniformType::Vec4, 1);
	}


	void Update(float deltaTime) override
	{
		m_currentView = m_firstView - 1;//////

		for (size_t i = m_debugMeshesStartDynamic; i < m_debugObjects.GetSize(); ++i)
		{
			DebugObject& dMesh = m_debugObjects[i];
			dMesh.lifetime -= deltaTime;
			if (dMesh.lifetime <= 0.0f)
			{
				DestroyMeshData(dMesh.mesh.renderDataHandle);
				m_allocator.Deallocate(dMesh.mesh.verticesData);
				m_allocator.Deallocate(dMesh.mesh.indicesData);
				m_engine.GetResourceManagement()->UnloadResource(Material::RESOURCE_TYPE, dMesh.mesh.material);
				m_debugObjects.Erase(i);
			}
		}
	}


	const char* GetName() const override { return "renderer"; }


	Scene* GetScene(worldId world) const override
	{
		RenderSceneImpl** scene;
		if (m_scenes.Find(world, scene))
			return *scene;
		else
			return nullptr;
	}

	const ComponentBase** GetComponents(uint& count) const override
	{
		static Component<RenderScene, &RenderScene::AddModel, &RenderScene::RemoveModel, &RenderScene::HasModel> model("Model");
		static Component<RenderScene, &RenderScene::AddCamera, &RenderScene::RemoveCamera, &RenderScene::HasCamera> camera("Camera");
		static Component<RenderScene, &RenderScene::AddDirectionalLight, &RenderScene::RemoveDirectionalLight, &RenderScene::HasDirectionalLight> dirLight("Directional light");

		static const ComponentBase* components[] = { &model, &camera, &dirLight };
		count = sizeof(components) / sizeof(components[0]);

		return components;
	}

	SceneEditor* GetEditor() override
	{
		return &m_sceneEditor;
	}


	void OnWorldAdded(worldId world) override
	{
		RenderSceneImpl* scene = NEW_OBJECT(m_allocator, RenderSceneImpl)(m_allocator, *this);
		m_scenes.Insert(world, scene);
	}

	void OnWorldRemoved(worldId world) override
	{
		m_scenes.Erase(world);
	}


	MaterialManager& GetMaterialManager() override { return m_materialManager; }
	ShaderManager& GetShaderManager() override { return m_shaderManager; }
	ModelManager& GetModelManager() override { return m_modelManager; }
	TextureManager& GetTextureManager() override { return m_textureManager; }


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
		if (mesh.varyings & ShaderVarying_Tangent)
		{
			meshData.vertex_decl.add(bgfx::Attrib::Tangent, 3, bgfx::AttribType::Float);
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


	void AddDebugLine(Vector3 from, Vector3 to, Color color, float width, float lifetime) override
	{
		DebugObject& dObject = m_debugObjects.PushBack();

		dObject.mesh.varyings = ShaderVarying_Position | ShaderVarying_Color0;
		dObject.mesh.verticesCount = 4;
		const size_t bufferSize = (sizeof(Vector3) + sizeof(Color)) * dObject.mesh.verticesCount;
		dObject.mesh.verticesData = (u8*)m_allocator.Allocate(bufferSize, alignof(float));
		u8* vData = dObject.mesh.verticesData;

		Vector3 LineDir = to - from;
		float lineLength = LineDir.Length();
		LineDir = LineDir / lineLength;
		float lineLengthHalf = lineLength * 0.5f;
		float widthHalf = width * 0.5f;

		Vector3 from1(0.0f, -widthHalf, -lineLengthHalf);
		Vector3 from2(0.0f, widthHalf, -lineLengthHalf);
		Vector3 to1(0.0f, -widthHalf, lineLengthHalf);
		Vector3 to2(0.0f, widthHalf, lineLengthHalf);

		memory::Copy(vData, &from1, 3 * sizeof(float));
		vData += 3 * sizeof(float);
		memory::Copy(vData, &color, sizeof(Color));
		vData += sizeof(Color);
		memory::Copy(vData, &to1, 3 * sizeof(float));
		vData += 3 * sizeof(float);
		memory::Copy(vData, &color, sizeof(Color));
		vData += sizeof(Color);

		memory::Copy(vData, &from2, 3 * sizeof(float));
		vData += 3 * sizeof(float);
		memory::Copy(vData, &color, sizeof(Color));
		vData += sizeof(Color);
		memory::Copy(vData, &to2, 3 * sizeof(float));
		vData += 3 * sizeof(float);
		memory::Copy(vData, &color, sizeof(Color));
		vData += sizeof(Color);

		dObject.mesh.indicesCount = 2;
		size_t count = dObject.mesh.indicesCount * 3;//triangles
		dObject.mesh.indicesData = (u16*)m_allocator.Allocate(count * sizeof(u16), alignof(u16));
		const u16 iData[] = { 0, 2, 1, 1, 2, 3 };
		memory::Copy(dObject.mesh.indicesData, iData, count * sizeof(u16));

		Path materialPath("materials/debug.material");
		dObject.mesh.material = m_engine.GetResourceManagement()->GetManager(Material::RESOURCE_TYPE)->Load(materialPath);
		dObject.mesh.renderDataHandle = CreateMeshData(dObject.mesh);
		dObject.lifetime = lifetime;

		dObject.transform.position = (from + to) * 0.5f;
		Vector3 rotAxis = Vector3::Cross(LineDir, Vector3::AXIS_Z);
		rotAxis.Normalize();
		float rotAngle = acosf(Vector3::Dot(LineDir, Vector3::AXIS_Z));
		dObject.transform.rotation = Quaternion(rotAxis, rotAngle);

		if (lifetime == -1.0f)
		{
			m_debugObjects.Swap(m_debugMeshesStartDynamic, m_debugObjects.GetSize() - 1);
			m_debugMeshesStartDynamic++;
		}
		else
		{
			dObject.lifetime *= 1000.0f;
		}
	}


	void AddDebugSquare(Vector3 position, Color color, float size, float lifetime) override
	{
		DebugObject& dObject = m_debugObjects.PushBack();

		dObject.mesh.varyings = ShaderVarying_Position | ShaderVarying_Color0;
		dObject.mesh.verticesCount = 4;
		const size_t bufferSize = (sizeof(Vector3) + sizeof(Color)) * dObject.mesh.verticesCount;
		dObject.mesh.verticesData = (u8*)m_allocator.Allocate(bufferSize, alignof(float));
		u8* vData = dObject.mesh.verticesData;

		float sizeHalf = size * 0.5f;

		Vector3 v1(-sizeHalf, sizeHalf, 0.0f);
		Vector3 v2(sizeHalf, sizeHalf, 0.0f);
		Vector3 v3(-sizeHalf, -sizeHalf, 0.0f);
		Vector3 v4(sizeHalf, -sizeHalf, 0.0f);

		memory::Copy(vData, &v1, 3 * sizeof(float));
		vData += 3 * sizeof(float);
		memory::Copy(vData, &color, sizeof(Color));
		vData += sizeof(Color);
		memory::Copy(vData, &v3, 3 * sizeof(float));
		vData += 3 * sizeof(float);
		memory::Copy(vData, &color, sizeof(Color));
		vData += sizeof(Color);

		memory::Copy(vData, &v2, 3 * sizeof(float));
		vData += 3 * sizeof(float);
		memory::Copy(vData, &color, sizeof(Color));
		vData += sizeof(Color);
		memory::Copy(vData, &v4, 3 * sizeof(float));
		vData += 3 * sizeof(float);
		memory::Copy(vData, &color, sizeof(Color));
		vData += sizeof(Color);

		dObject.mesh.indicesCount = 2;
		size_t count = dObject.mesh.indicesCount * 3;//triangles
		dObject.mesh.indicesData = (u16*)m_allocator.Allocate(count * sizeof(u16), alignof(u16));
		const u16 iData[] = { 0, 2, 1, 1, 2, 3 };
		memory::Copy(dObject.mesh.indicesData, iData, count * sizeof(u16));

		Path materialPath("materials/debug.material");
		dObject.mesh.material = m_engine.GetResourceManagement()->GetManager(Material::RESOURCE_TYPE)->Load(materialPath);
		dObject.mesh.renderDataHandle = CreateMeshData(dObject.mesh);
		dObject.lifetime = lifetime;

		dObject.transform.position = position;

		if (lifetime == -1.0f)
		{
			m_debugObjects.Swap(m_debugMeshesStartDynamic, m_debugObjects.GetSize() - 1);
			m_debugMeshesStartDynamic++;
		}
		else
		{
			dObject.lifetime *= 1000.0f;
		}
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
		const RenderScene::CameraItem* cameraItem = scene->GetCameraData(camera);
		if (cameraItem != nullptr)
		{
			const Camera& cam = cameraItem->camera;
			const Transform& camTrans = world.GetEntityTransform(cameraItem->entity);

			if (cam.type == Camera::Type::Perspective)
			{
				proj.SetPerspective(cam.fov, cam.aspect, cam.nearPlane, cam.farPlane, bgfx::getCaps()->homogeneousDepth);
				Vector4 foo = proj * Vector4(2, 0, 2, 1);
				foo = foo / foo.w;
				foo = foo;
			}
			else if (cam.type == Camera::Type::Orthogonal)
			{
				float halfW = cam.screenWidth * 0.5f;
				float halfH = cam.screenHeight * 0.5f;
				proj.SetOrthogonal(-halfW, halfW, -halfH, halfH, cam.nearPlane, cam.farPlane, 0.0f, bgfx::getCaps()->homogeneousDepth);
			}

			Vector4 eye = Vector4(camTrans.position, 1);
			Vector4 at = Vector4(Quaternion::Multiply(camTrans.rotation, Vector3::AXIS_Z), 0) + eye;
			view.SetLookAt(eye, at, Vector4::AXIS_Y);
			eye.w = (float)cam.type;

			bgfx::setUniform(m_cameraPos, &eye);
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

	void RenderModels(World& world)
	{
		const RenderSceneImpl* scene = (RenderSceneImpl*)GetScene(world.GetId());
		if(scene->GetDirectionalLightsCount() > 0)
		{
			const RenderScene::DirectionalLightItem* dirLight = scene->GetDirectionalLights();
			Transform& dirLightTrans = world.GetEntityTransform(dirLight->entity);
			Vector4 dir = Vector4(-dirLightTrans.position, 0);
			dir.Normalize();
			bgfx::setUniform(m_dirLightsDirs, &dir);
			bgfx::setUniform(m_dirLightsColor, &dirLight->light);
		}

		size_t count;
		const RenderSceneImpl::ModelItem* models = scene->GetModels(count);
		for (size_t i = 0; i < count; ++i)
		{

			Transform& trans = world.GetEntityTransform(models[i].entity);
			Matrix44 mtx = trans.ToMatrix44();
			bgfx::setTransform(&mtx.m11);// Set model matrix for rendering.

			if (models[i].model == INVALID_RESOURCE_HANDLE) continue; //TODO: can't do this comparsion
			const Model* model = (Model*)m_modelManager.GetResource(models[i].model);
			if (model->GetState() == Resource::State::Ready)
			{
				for (const Mesh& mesh : model->meshes)
				{
					const Material* material = (Material*)m_materialManager.GetResource(mesh.material);
					if (material->GetState() == Resource::State::Ready)
					{
						MeshData& meshData = m_meshData.Get((u64)mesh.renderDataHandle);
						bgfx::setVertexBuffer(0, meshData.vertexBufferHandle);
						bgfx::setIndexBuffer(meshData.indexBufferHandle);

						MaterialData& materialData = m_materialData.Get((u64)material->renderDataHandle);
						for(int i = 0; i < materialData.textureCount; ++i)
						{
							const Texture* texture = (Texture*)m_textureManager.GetResource(material->textures[i]);
							TextureData& texData = m_textureData.Get((u64)texture->renderDataHandle);
							bgfx::setTexture(i, materialData.textureUniforms[i], texData.handle);
						}

						uint64_t state = BGFX_STATE_DEFAULT;// BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_MSAA;
						bgfx::setState(state);

						const Shader* shader = (Shader*)m_shaderManager.GetResource(material->shader);
						ShaderData& shaderData = m_shaderData.Get((u64)shader->renderDataHandle);
						bgfx::submit(m_currentView, shaderData.handle);
					}
				}
			}
		}
	}

	void RenderDebug() override
	{
		for (int i = 0; i < m_debugObjects.GetSize(); ++i)
		{
			const Mesh& mesh = m_debugObjects[i].mesh;
			const Material* material = (Material*)m_materialManager.GetResource(mesh.material);
			if (material->GetState() == Resource::State::Ready)
			{
				Matrix44 transform = m_debugObjects[i].transform.ToMatrix44();
				bgfx::setTransform(&transform);

				MeshData& meshData = m_meshData.Get((u64)mesh.renderDataHandle);
				bgfx::setVertexBuffer(0, meshData.vertexBufferHandle);
				bgfx::setIndexBuffer(meshData.indexBufferHandle);

				uint64_t state = BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_MSAA | BGFX_STATE_DEPTH_TEST_LESS;
				bgfx::setState(state);

				const Shader* shader = (Shader*)m_shaderManager.GetResource(material->shader);
				ShaderData& shaderData = m_shaderData.Get((u64)shader->renderDataHandle);
				bgfx::submit(m_currentView, shaderData.handle);
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

	ShaderInternalManager m_shaderInternalManager;
	ShaderManager m_shaderManager;
	MaterialManager m_materialManager;
	ModelManager m_modelManager;
	TextureManager m_textureManager;

	RenderSceneEditorImpl m_sceneEditor;
	///////////////////// DEBUG STUFF
	resourceHandle m_debugShader = INVALID_RESOURCE_HANDLE;
	Array<DebugObject> m_debugObjects;
	size_t m_debugMeshesStartDynamic = 0;
	/////////////////////
	u32 m_width = 0;
	u32 m_height = 0;
	/////////////////////
	bgfx::ViewId m_firstView = 1;//TODO
	bgfx::ViewId m_currentView = m_firstView - 1;//TODO
	HandleArray<FrameBuffer, u16> m_framebuffers;
	Array<FramebufferHandle> m_screenSizeFrameBuffers;

	bgfx::UniformHandle m_cameraPos;
	bgfx::UniformHandle m_dirLightsDirs;
	bgfx::UniformHandle m_dirLightsColor;
};


RenderSystem* RenderSystem::Create(Engine& engine)
{
	return NEW_OBJECT(engine.GetAllocator(), RenderSystemImpl)(engine);
}

void RenderSystem::Destroy(RenderSystem* system)
{
	Allocator& allocator = system->GetEngine().GetAllocator();
	DELETE_OBJECT(allocator, system);
}

}