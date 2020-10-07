#include "script.h"

#include "core/engine.h"
#include "core/allocator.h"
#include "core/allocators.h"
#include "core/containers/associative_array.h"
#include "core/logs.h"
#include "core/string.h"
#include "core/hashes.h"
#include "core/file/blob.h"

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


struct DummyScriptClass :ScriptClassBase
{
	void Init(class Engine& engine) override {}
	void Deinit() override {}
	void Update(float deltaTime) override {}
};
static DummyScriptClass s_dummyScript;


struct ScriptItem
{
	Entity entity;
	bool active;
	ScriptScene::ScriptData data;
	ScriptClassBase* script = nullptr;
};


class ScriptSceneImpl : public ScriptScene
{
public:
	ScriptSceneImpl(Allocator& allocator, ScriptSystem& scriptSystem)
		: m_allocator(allocator)
		, m_scriptSystem(scriptSystem)
		, m_scripts(m_allocator)
	{
	}

	~ScriptSceneImpl() override
	{
		for (auto& script : m_scripts)
		{
			if (*script.data.className != '\0')
				DELETE_OBJECT(m_allocator, script.script);
		}
	}

	virtual void Serialize(class OutputBlob& serializer) const override
	{
		SerializeScripts(serializer);
	}

	virtual void Deserialize(class InputBlob& serializer) override
	{
		DeserializeScripts(serializer);
	}

	void Clear() override {}

	void Update(float deltaTime) override
	{}


	void AddScript(Entity entity) override
	{
		ScriptItem* item = m_scripts.Insert(entity, { entity, false, {}, &s_dummyScript });
		ASSERT2(item != nullptr, "entity already has script component");
	}

	void RemoveScript(Entity entity) override
	{
		m_scripts.Erase(entity);
	}

	bool HasScript(Entity entity) const override
	{
		ScriptItem* script;
		return m_scripts.Find(entity, script);
	}

	ScriptData* GetScriptData(Entity entity) const override
	{
		ScriptItem* item = nullptr;
		if (m_scripts.Find(entity, item))
		{
			return &item->data;
		}
		else
		{
			Log(LogType::Warning, "Script component for entity %d was not found", (u64)entity);
			return nullptr;
		}
	}

	ScriptClassBase* InstantiateScript(const char* className)
	{
		u32 newNameHash = crc32_string(className);
		ScriptClassRegistry* firstReg = GetScriptRegistries();
		while (firstReg != nullptr)
		{
			if (firstReg->nameHash == newNameHash)
				return firstReg->creator(m_allocator);

			firstReg = firstReg->next;
		}
		return nullptr;
	}

	void SetScriptData(Entity entity, const ScriptData& data) override
	{
		ScriptItem* item;
		if (m_scripts.Find(entity, item))
		{
			if (string::Compare(item->data.className, data.className) != 0)
			{
				string::Copy(item->data.className, data.className);
				ScriptClassBase* script = InstantiateScript(data.className);
				if (script != nullptr) {
					item->script = script;
					script->Init(m_scriptSystem.GetEngine());
				}
				else {
					item->active = false;
					Log(LogType::Warning, "Could not instantiate script \"%s\"", data.className);
				}
			}
		}
		else
		{
			Log(LogType::Warning, "Script component for entity %d was not found", (u64)entity);
		}
	}

	void SerializeScripts(OutputBlob& serializer) const
	{
		serializer.Write((u64)m_scripts.GetSize());
		for (const ScriptItem& script : m_scripts)
		{
			serializer.Write(script.entity);
			serializer.Write(script.active);
			serializer.Write(script.data.className, string::Length(script.data.className) + 1);
		}
	}

	void DeserializeScripts(InputBlob& serializer)
	{
		StackAllocator<128> allocator;

		u64 count;
		serializer.Read(count);
		for (u64 i = 0; i < count; ++i)
		{
			Entity entity;
			serializer.Read(entity);
			ScriptItem* script = m_scripts.Insert(entity, { entity });
			serializer.Read(script->active);
			String className(allocator);
			serializer.Read(className);
			ScriptData data;
			string::Copy(data.className, className.Cstr(), 64);
			SetScriptData(entity, data);
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
	Allocator& m_allocator;
	ScriptSystem& m_scriptSystem;

	AssociativeArray<Entity, ScriptItem> m_scripts;
	size_t m_activeScriptsCount = 0;
};

//----------------------------------------------------------------------------------


class ScriptSceneEditorImpl : public ScriptSceneEditor
{
public:
	ScriptSceneEditorImpl(ScriptSystem& system) : m_system(system) {}
	~ScriptSceneEditorImpl() override {}

	void EditComponent(EditorInterface& editor, const ComponentBase& component, worldId world, Entity entity) override
	{

	}

private:
	ScriptSystem& m_system;
};

//----------------------------------------------------------------------------------


class ScriptSystemImpl : public ScriptSystem
{
	enum class Version : u32
	{
		First = 0,
		Latest
	};

public:
	ScriptSystemImpl(Engine& engine)
		: m_allocator(engine.GetAllocator())
		, m_engine(engine)
		, m_scenes(m_allocator, &HashWorldId)
		, m_sceneEditor(*this)
	{}

	~ScriptSystemImpl() override
	{
		for (const auto& scene : m_scenes)
			DELETE_OBJECT(m_allocator, scene.value);
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
			ScriptSceneImpl* scene = NEW_OBJECT(m_allocator, ScriptSceneImpl)(m_allocator, *this);
			scene->Deserialize(serializer);
			m_scenes.Insert(world, scene);
		}
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
		renderScene->AddDirectionalLight(dirLight);
		renderScene->SetDirectionalLightData(dirLight, { dirLight, dirLightData });

		Path modelPath;

		int i = 0;
		for (unsigned yy = 0; yy < 1/*11*/; ++yy)
		{
			for (unsigned xx = 0; xx < 1/*11*/; ++xx)
			{
				m_entities[i] = world->CreateEntity();
				Transform& trans = world->GetEntityTransform(m_entities[i]);
				renderScene->AddModel(m_entities[i]);
				float scale = 1.0f;
				if (i % 2 == 0)
				{
					modelPath = Path("models/square.model");
					scale = 10;
				}
				else
				{
					modelPath = Path("models/sphere.model");
					scale = 0.09f;
				}
				renderScene->SetModelData(m_entities[i], { modelPath });

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
		renderSystem->AddDebugLine(Vector3(10, 0, 25), Vector3(-10, 0, -5), Color(0, 255, 255, 255), 0.8f, -1.0f);
		renderSystem->AddDebugSquare(Vector3(5, -5, 15), Color(0, 128, 128, 255), 0.8f, -1.0f);
	}

	void Update(float deltaTime) override
	{
		for (auto& sceneNode : m_scenes)
		{
			const ScriptItem* scripts = sceneNode.value->GetScripts();
			for (int i = 0; i < sceneNode.value->GetScriptsCount(); ++i)
			{
				if(scripts[i].active)//TODO: change from map to array and keep inactive on end ?
					scripts[i].script->Update(deltaTime);
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

	Scene* GetScene(worldId world) const override
	{
		ScriptSceneImpl** scene;
		if (m_scenes.Find(world, scene))
			return *scene;
		else
			return nullptr;
	}

	const ComponentBase** GetComponents(uint& count) const override
	{
		static Component<ScriptScene, &ScriptScene::AddScript, &ScriptScene::RemoveScript, &ScriptScene::HasScript> script("script");

		static const ComponentBase* components[] = { &script };

		count = sizeof(components) / sizeof(components[0]);
		return components;
	}

	SceneEditor* GetEditor() override
	{
		return &m_sceneEditor;
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

	ScriptSceneEditorImpl m_sceneEditor;

	worldId m_world = INVALID_WORLD_ID;
	Entity m_entities[121];
};


ScriptSystem* ScriptSystem::Create(Engine& engine)
{
	return NEW_OBJECT(engine.GetAllocator(), ScriptSystemImpl)(engine);
}

void ScriptSystem::Destroy(ScriptSystem* system)
{
	Allocator& allocator = system->GetEngine().GetAllocator();
	DELETE_OBJECT(allocator, system);
}


}