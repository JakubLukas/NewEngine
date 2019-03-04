#pragma once

#include "int.h"
#include "iallocator.h"
#include "containers/array.h"

#include "core/world/world.h"


namespace Veng
{

class EditorInterface;


enum class componentHandle : u8 {};
const componentHandle INVALID_COMPONENT_HANDLE = (componentHandle)-1;

struct ComponentInfo
{
	enum class ValueType : u8
	{
		ResourceHandle,
		Int,
		Float,
		Angle,
		String,
		Vector3,
		Color,
		//Text,
		//Array,
	};

	struct Value
	{
		ValueType type;
		const char* name;
	};

	ComponentInfo(IAllocator& allocator) : values(allocator) {}

	componentHandle handle;
	const char* name;
	Array<Value> values;
	size_t dataSize = 0;
};


class IScene
{
public:
	virtual ~IScene() {}

	virtual void Update(float deltaTime) = 0;

	virtual size_t GetComponentCount() const = 0;
	virtual const ComponentInfo* GetComponentInfos() const = 0;
	virtual const ComponentInfo* GetComponentInfo(componentHandle handle) const = 0;
	virtual componentHandle GetComponentHandle(const char* name) const = 0;

	virtual void AddComponent(componentHandle handle, Entity entity) = 0;
	virtual void RemoveComponent(componentHandle handle, Entity entity) = 0;
	virtual bool HasComponent(componentHandle handle, Entity entity) const = 0;
	virtual void EditComponent(EditorInterface* editor, componentHandle handle, Entity entity) = 0;
	//virtual void GetComponentData(componentHandle handle, Entity entity, void* buffer) const = 0;
	//virtual void SetComponentData(componentHandle handle, Entity entity, void* data) = 0;
};


class ISystem
{
public:
	virtual ~ISystem() {}

	virtual void Init() = 0;

	virtual void Update(float deltaTime) = 0;
	virtual const char* GetName() const = 0;

	virtual IScene* GetScene(worldId world) const = 0;

	virtual void WorldAdded(worldId world) = 0;
	virtual void WorldRemoved(worldId world) = 0;
};


}