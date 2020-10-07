#pragma once

#include "core/world/world.h"


namespace Veng
{

class EditorInterface;


struct PropertyBase
{

};


struct ComponentBase
{
	ComponentBase(const char* name);

	virtual void Create(class Scene& scene, Entity entity) const = 0;
	virtual void Destroy(class Scene& scene, Entity entity) const = 0;
	virtual bool Has(class Scene& scene, Entity entity) const = 0;

	const char* name;
	u32 type;
};

template<typename ObjectType, void(ObjectType::*createFn)(Entity), void(ObjectType::*destroyFn)(Entity), bool(ObjectType::*hasFn)(Entity) const>
struct Component : ComponentBase
{
	Component(const char* name) : ComponentBase(name) {}

	void Create(class Scene& scene, Entity entity) const override
	{
		(static_cast<ObjectType&>(scene).*createFn)(entity);
	}
	void Destroy(class Scene& scene, Entity entity) const override
	{
		(static_cast<ObjectType&>(scene).*destroyFn)(entity);
	}
	bool Has(class Scene& scene, Entity entity) const override
	{
		return (static_cast<ObjectType&>(scene).*hasFn)(entity);
	}
};



class Scene
{
public:
	virtual ~Scene() {}

	virtual void Serialize(class OutputBlob& serializer) const = 0;
	virtual void Deserialize(class InputBlob& serializer) = 0;

	virtual void Clear() = 0;
	virtual void Update(float deltaTime) = 0;
};


class SceneEditor
{
public:
	virtual ~SceneEditor() {}

	virtual void EditComponent(EditorInterface& editor, const ComponentBase& component, worldId world, Entity entity) = 0;
};


class System
{
public:
	virtual ~System() {}

	virtual u32 GetVersion() const { return 0; }
	virtual void Serialize(class OutputBlob& serializer) const = 0;
	virtual void Deserialize(class InputBlob& serializer) = 0;

	virtual void Init() = 0;

	virtual void Update(float deltaTime) = 0;
	virtual const char* GetName() const = 0;

	virtual Scene* GetScene(worldId world) const = 0;
	virtual const ComponentBase** GetComponents(uint& count) const = 0;
	virtual SceneEditor* GetEditor() = 0;

	virtual void OnWorldAdded(worldId world) = 0;
	virtual void OnWorldRemoved(worldId world) = 0;
};


}