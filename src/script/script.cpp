#include "script.h"

#include "core/engine.h"
#include "core/iallocator.h"
#include "core/allocators.h"
#include "core/containers/associative_array.h"
#include "core/logs.h"
#include "core/string.h"

#include "editor/editor_interface.h"

#include "core/world/world.h"//////////////
#include "renderer/renderer.h"////////////
#include "renderer/resource_managers/model_manager.h"/////////////////////////////
#include "core/math/math.h"///////////////////
#include "core/file/path.h"/////////////
#include "core/math/quaternion.h"///////////
#include "core/math/matrix.h"//////////////
#include "core/time.h"///////////////


namespace Veng
{


static u32 HashWorldId(const worldId& world)
{
	return HashU32((u32)world);
}


static void DummyScriptUpdate(void* data, Engine& engine, float deltaTime) {}


componentHandle ScriptScene::GetComponentHandle(Component comp)
{
	return (componentHandle)comp;
}


class ScriptSceneImpl : public ScriptScene
{
public:
	ScriptSceneImpl(IAllocator& allocator, ScriptSystem& scriptSystem)
		: m_allocator(allocator)
		, m_scriptSystem(scriptSystem)
		, m_componentInfos(m_allocator)
		, m_scripts(m_allocator)
	{
		ComponentInfo* compInfoModel;

		compInfoModel = &m_componentInfos.EmplaceBack();
		compInfoModel->handle = ScriptScene::GetComponentHandle(Component::Script);
		compInfoModel->name = "script";
	}

	~ScriptSceneImpl() override {}

	void Update(float deltaTime) override
	{}

	size_t GetComponentCount() const override { return m_componentInfos.GetSize(); }

	const ComponentInfo* GetComponentInfos() const override { return m_componentInfos.Begin(); }

	const ComponentInfo* GetComponentInfo(componentHandle handle) const override { return &m_componentInfos[(size_t)handle]; }

	componentHandle GetComponentHandle(const char* name) const override
	{
		if (string::Equal(name, "model"))
		{
			return ScriptScene::GetComponentHandle(Component::Script);
		}
		else
		{
			return INVALID_COMPONENT_HANDLE;
		}
	}

	void AddComponent(componentHandle handle, Entity entity) override
	{
		switch ((u8)handle)
		{
		case (u8)ScriptScene::Component::Script:
		{
			ScriptItem* item = m_scripts.Insert(entity, { entity, true, ScriptClass() });
			ASSERT2(item != nullptr, "entity already has script component");
			item->script.updateFunction = DummyScriptUpdate;
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
		case (u8)ScriptScene::Component::Script:
		{
			m_scripts.Erase(entity);
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
			ScriptItem* script;
			return m_scripts.Find(entity, script);
		}
		default:
			ASSERT2(false, "Unrecognized componentHandle");
			return false;
		}
	}

	void EditComponent(EditorInterface* editor, componentHandle handle, Entity entity) override
	{
		switch ((u8)handle)
		{
		case (u8)RenderScene::Component::Model:
		{
			ScriptItem* scriptItem;
			if (m_scripts.Find(entity, scriptItem))
			{
				char strBuffer[36] = "Script edit";
				editor->EditString("Script edit", strBuffer, 36);
			}
			else
			{
				Log(LogType::Warning, "Script component for entity %d was not found", (u64)entity);
			}
			break;
		}
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}
	}

	void* GetComponentData(componentHandle handle, Entity entity) const override
	{
		switch ((u8)handle)
		{
		case (u8)ScriptScene::Component::Script:
		{
			ScriptItem* item;
			if (m_scripts.Find(entity, item))
			{
				return &item->script;
			}
			else
			{
				Log(LogType::Warning, "Script component for entity %d was not found", (u64)entity);
			}
			break;
		}
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}

		return nullptr;
	}

	void SetComponentData(componentHandle handle, Entity entity, void* data) override
	{
		switch ((u8)handle)
		{
		case (u8)ScriptScene::Component::Script:
		{
			ScriptClass* dataScript = (ScriptClass*)data;

			ScriptItem* item;
			if (m_scripts.Find(entity, item))
			{
				//TODO: set script memory
				item->script.updateFunction = dataScript->updateFunction;
			}
			else
			{
				Log(LogType::Warning, "Script component for entity %d was not found", (u64)entity);
			}
			break;
		}
		default:
			ASSERT2(false, "Unrecognized componentHandle");
		}
	}

	void SetScriptActive(Entity entity, bool active) override
	{
		ScriptItem* item;
		if (m_scripts.Find(entity, item))
		{
			item->active = active;
		}
		else
		{
			Log(LogType::Warning, "Script component for entity %d was not found", (u64)entity);
		}
	}

	size_t GetScriptsCount() const { return m_scripts.GetSize(); }
	const ScriptItem* GetScripts() const { return m_scripts.GetValues(); }

private:
	IAllocator& m_allocator;
	ScriptSystem& m_scriptSystem;
	Array<ComponentInfo> m_componentInfos;

	AssociativeArray<Entity, ScriptItem> m_scripts;
	size_t m_activeScriptsCount = 0;
};


class ScriptSystemImpl : public ScriptSystem
{
public:
	ScriptSystemImpl(Engine& engine)
		: m_allocator(engine.GetAllocator())
		, m_engine(engine)
		, m_scenes(m_allocator, &HashWorldId)
	{
	}
	~ScriptSystemImpl() override
	{
		for (const auto& scene : m_scenes)
			DELETE_OBJECT(m_allocator, scene.value);
	}

	void Init() override
	{
		m_world = m_engine.AddWorld();
		World* world = m_engine.GetWorld(m_world);
		RenderSystem* renderSystem = (RenderSystem*)m_engine.GetSystem("renderer");
		RenderScene* renderScene = static_cast<RenderScene*>(renderSystem->GetScene(m_world));

		Entity dirLight = world->CreateEntity();
		Transform& camTrans = world->GetEntityTransform(dirLight);
		camTrans.position = Vector3(-30, 30, -10);

		DirectionalLight dirLightData;
		dirLightData.diffuseColor = Color(255, 255, 255, 255);
		dirLightData.specularColor = Color(255, 255, 255, 255);
		renderScene->AddComponent(RenderScene::GetComponentHandle(RenderScene::Component::DirectionalLight), dirLight);
		renderScene->SetComponentData(RenderScene::GetComponentHandle(RenderScene::Component::DirectionalLight), dirLight, &dirLightData);

		resourceHandle modelHandle = INVALID_RESOURCE_HANDLE;

		int i = 0;
		for (unsigned yy = 0; yy < 1/*11*/; ++yy)
		{
			for (unsigned xx = 0; xx < 1/*11*/; ++xx)
			{
				m_entities[i] = world->CreateEntity();
				Transform& trans = world->GetEntityTransform(m_entities[i]);
				renderScene->AddComponent(RenderScene::GetComponentHandle(RenderScene::Component::Model), m_entities[i]);
				float scale = 1.0f;
				if (i % 2 == 0)
				{
					modelHandle = renderSystem->GetModelManager().Load(Path("models/square.model"));
					scale = 10;
				}
				else
				{
					modelHandle = renderSystem->GetModelManager().Load(Path("models/sphere.model"));
					scale = 0.09f;
				}
				renderScene->SetComponentData(RenderScene::GetComponentHandle(RenderScene::Component::Model), m_entities[i], &modelHandle);

				Quaternion rot = Quaternion::IDENTITY;
				//rot = rot * Quaternion(Vector3::AXIS_X, xx*0.21f);
				//rot = rot * Quaternion(Vector3::AXIS_Y, yy*0.37f);
				Vector3 pos = {0,0,35
					//-15.0f + float(xx) * 3.0f,
					//-15.0f + float(yy) * 3.0f,
					//35.0f
				};
				trans = Transform(rot, pos, scale);
				i++;
			}
		}
		renderSystem->AddDebugLine(Vector3(10, 0, 25), Vector3(-10, 0, -5), Color(0, 255, 255, 255), 0.1f, -1.0f);
	}

	void Update(float deltaTime) override
	{
		for (auto& sceneNode : m_scenes)
		{
			const ScriptScene::ScriptItem* scripts = sceneNode.value->GetScripts();
			for (int i = 0; i < sceneNode.value->GetScriptsCount(); ++i)
			{
				if(scripts[i].active)// BOOOOOOOOOOOOOOOOOOOOOOOO
					scripts[i].script.updateFunction(scriptMemory, m_engine, deltaTime);
			}
		}

		// DUMMY GAMEPLAY SCRIPT
		Quaternion rot = Quaternion::IDENTITY;
		rot = rot * Quaternion(Vector3::AXIS_X, SecFromMSec(deltaTime));

		World* world = m_engine.GetWorld(m_world);
		
		for (int i = 0; i < 121; ++i)
		{
			//Transform& trans = world->GetEntityTransform(m_entities[i]);
			//trans.rotation = trans.rotation * rot;
		}
	}


	const char* GetName() const override { return "script"; }

	IScene* GetScene(worldId world) const override
	{
		ScriptSceneImpl** scene;
		if (m_scenes.Find(world, scene))
			return *scene;
		else
			return nullptr;
	}
	void OnWorldAdded(worldId world) override
	{
		ScriptSceneImpl* scene = NEW_OBJECT(m_allocator, ScriptSceneImpl)(m_allocator, *this);
		m_scenes.Insert(world, scene);
	}
	void OnWorldRemoved(worldId world) override
	{
		m_scenes.Erase(world);
	}

	Engine& GetEngine() const override { return m_engine; }

private:
	ProxyAllocator m_allocator;//must be first
	Engine& m_engine;
	HashMap<worldId, ScriptSceneImpl*> m_scenes;

	void* scriptMemory = nullptr;

	worldId m_world;
	Entity m_entities[121];
};


ScriptSystem* ScriptSystem::Create(Engine& engine)
{
	return NEW_OBJECT(engine.GetAllocator(), ScriptSystemImpl)(engine);
}

void ScriptSystem::Destroy(ScriptSystem* system)
{
	IAllocator& allocator = system->GetEngine().GetAllocator();
	DELETE_OBJECT(allocator, system);
}


}