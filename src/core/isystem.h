#pragma once

#include "int.h"
#include "iallocator.h"
#include "containers/array.h"

#include "core/world/world.h"


namespace Veng
{

enum class componentHandle : u8 {};

struct ComponentInfo
{
	enum class ValueType : u8
	{
		ResourceHandle,
		Int,
		Float,
		String,
		Text,
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
};


class IScene
{
public:
	virtual ~IScene() {}

	virtual void Update(float deltaTime) = 0;

	virtual size_t GetComponentCount() const = 0;
	virtual const ComponentInfo* GetComponents() const = 0;
	virtual const ComponentInfo* GetComponentInfo(componentHandle handle) const = 0;

	virtual void AddComponent(componentHandle handle, Entity entity, worldId world) = 0;
	virtual void RemoveComponent(componentHandle handle, Entity entity, worldId world) = 0;
	virtual bool HasComponent(componentHandle handle, Entity entity, worldId world) const = 0;
	virtual void* GetComponentData(componentHandle handle, Entity entity, worldId world) const = 0;
	virtual void SetComponentData(componentHandle handle, Entity entity, worldId world, void* data) = 0;
};


class ISystem
{
public:
	virtual ~ISystem() {}

	virtual void Init() = 0;

	virtual void Update(float deltaTime) = 0;
	virtual const char* GetName() const = 0;

	virtual IScene* GetScene() const = 0;
};


}