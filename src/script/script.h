#pragma once

#include "core/isystem.h"


namespace Veng
{

class Engine;


class ScriptScene : public IScene
{
public:
	enum class Component : u8
	{
		Script = 0,

		Count
	};

	static componentHandle GetComponentHandle(Component comp);

	struct ScriptClass
	{
		void* data = nullptr;
		size_t dataSize = 0;
		using UpdateFunction = void(*)(void* data, Engine& engine, float deltaTime);

		UpdateFunction updateFunction;
	};

	struct ScriptItem
	{
		Entity entity;
		bool active;
		ScriptClass script;
	};

public:
	virtual ~ScriptScene() {}

	virtual void Update(float deltaTime) override = 0;

	virtual size_t GetComponentCount() const override = 0;
	virtual const ComponentInfo* GetComponentInfos() const override = 0;
	virtual const ComponentInfo* GetComponentInfo(componentHandle handle) const override = 0;
	virtual componentHandle GetComponentHandle(const char* name) const override = 0;

	virtual void AddComponent(componentHandle handle, Entity entity) override = 0;
	virtual void RemoveComponent(componentHandle handle, Entity entity) override = 0;
	virtual bool HasComponent(componentHandle handle, Entity entity) const override = 0;
	virtual void EditComponent(EditorInterface* editor, componentHandle handle, Entity entity) override = 0;
	virtual void* GetComponentData(componentHandle handle, Entity entity) const override = 0;
	virtual void SetComponentData(componentHandle handle, Entity entity, void* data) override = 0;

	virtual void SetScriptActive(Entity entity, bool active) = 0;
};


class ScriptSystem : public ISystem
{
public:
	static ScriptSystem* Create(Engine& engine);
	static void Destroy(ScriptSystem* system);

public:
	virtual ~ScriptSystem() override {}

	virtual void Init() override = 0;

	virtual void Update(float deltaTime) override = 0;
	virtual const char* GetName() const override = 0;

	virtual IScene* GetScene(worldId world) const override = 0;

	virtual void OnWorldAdded(worldId world) override = 0;
	virtual void OnWorldRemoved(worldId world) override = 0;

	virtual Engine& GetEngine() const = 0;
};


}