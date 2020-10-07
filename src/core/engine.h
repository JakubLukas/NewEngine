#pragma once

#include "allocator.h"
#include "system.h"

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
	static Engine* Create(Allocator& allocator);
	static void Destroy(Engine* engine, Allocator& allocator);

public:
	virtual bool Serialize(class OutputBlob& serializer) const = 0;
	virtual bool Deserialize(class InputBlob& serializer) = 0;

	virtual worldId AddWorld() = 0;
	virtual void RemoveWorld(worldId id) = 0;
	virtual World* GetWorld(worldId id) = 0;
	virtual size_t GetWorldCount() const = 0;
	virtual World* GetWorlds() = 0;

	virtual bool AddSystem(System* system) = 0;
	virtual bool RemoveSystem(const char* name) = 0;
	virtual System* GetSystem(const char* name) const = 0;
	virtual size_t GetSystemCount() const = 0;
	virtual System* const* GetSystems() const = 0;

	virtual bool AddResourceManager(ResourceManager& manager) = 0;
	virtual bool RemoveResourceManager(ResourceType type) = 0;
	virtual ResourceManager* GetResourceManager(ResourceType type) const = 0;
	virtual ResourceManagement* GetResourceManagement() const = 0;

	virtual void Update(float deltaTime) = 0;

	virtual Allocator& GetAllocator() const = 0;

	virtual FileSystem* GetFileSystem() const = 0;
	virtual InputSystem* GetInputSystem() const = 0;
};


}