#pragma once

#include "allocators.h"
#include "isystem.h"

#include "world/world.h"
#include "resource/resource.h"


namespace Veng
{

class FileSystem;
class InputSystem;
class ResourceManager;
class ResourceManagement;

typedef void* windowHandle;


class Engine
{
public:
	static Engine* Create(IAllocator& allocator);
	static void Destroy(Engine* engine, IAllocator& allocator);

public:
	virtual worldId AddWorld() = 0;
	virtual void RemoveWorld(worldId id) = 0;
	virtual World* GetWorld(worldId id) = 0;
	virtual size_t GetWorldCount() const = 0;
	virtual World* GetWorlds() = 0;

	virtual bool AddSystem(ISystem* system) = 0;
	virtual bool RemoveSystem(const char* name) = 0;
	virtual ISystem* GetSystem(const char* name) const = 0;
	virtual size_t GetSystemCount() const = 0;
	virtual ISystem* GetSystems() const = 0;

	virtual bool AddResourceManager(ResourceManager& manager) = 0;
	virtual bool RemoveResourceManager(ResourceType type) = 0;
	virtual ResourceManager* GetResourceManager(ResourceType type) const = 0;
	virtual ResourceManagement* GetResourceManagement() const = 0;

	virtual void Update(float deltaTime) = 0;

	virtual IAllocator& GetAllocator() const = 0;

	virtual FileSystem* GetFileSystem() const = 0;
	virtual InputSystem* GetInputSystem() const = 0;
};


}