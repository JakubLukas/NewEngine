#pragma once

#include "core/system.h"
#include "script_class.h"


namespace Veng
{

class Engine;


class ScriptScene : public Scene
{
public:
	struct ScriptData
	{
		char className[65] = { 0 };
	};

public:
	virtual ~ScriptScene() {}

	virtual void AddScript(Entity entity) = 0;
	virtual void RemoveScript(Entity entity) = 0;
	virtual bool HasScript(Entity entity) const = 0;
	virtual const ScriptData* GetScriptData(Entity entity) const = 0;
	virtual void SetScriptData(Entity entity, const ScriptData& data) = 0;

	virtual void SetScriptActive(Entity entity, bool active) = 0;
};


class ScriptSceneEditor : public SceneEditor
{
public:
	virtual ~ScriptSceneEditor() override {}

	virtual void EditComponent(EditorInterface& editor, const ComponentBase& component, worldId world, Entity entity) override = 0;
};


class ScriptSystem : public System
{
public:
	static ScriptSystem* Create(Engine& engine);
	static void Destroy(ScriptSystem* system);

public:
	virtual ~ScriptSystem() override {}

	virtual void Init() override = 0;

	virtual void Update(float deltaTime) override = 0;
	virtual const char* GetName() const override = 0;

	virtual Scene* GetScene(worldId world) const override = 0;
	virtual const ComponentBase** GetComponents(uint& count) const override = 0;
	virtual SceneEditor* GetEditor() override = 0;

	virtual void OnWorldAdded(worldId world) override = 0;
	virtual void OnWorldRemoved(worldId world) override = 0;

	virtual Engine& GetEngine() const = 0;
};


}