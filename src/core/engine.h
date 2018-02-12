#pragma once

#include "allocators.h"
#include "iplugin.h"


namespace Veng
{

class FileSystem;
class InputSystem;
class ResourceManagement;

typedef void* windowHandle;


class Engine
{
public:
	struct PlatformData
	{
		windowHandle windowHndl;
	};

public:
	static Engine* Create(IAllocator& allocator);
	static void Destroy(Engine* engine, IAllocator& allocator);

public:
	virtual void SetPlatformData(const PlatformData& data) = 0;
	virtual const PlatformData& GetPlatformData() const = 0;

	virtual bool AddPlugin(IPlugin* plugin) = 0;
	virtual void RemovePlugin(const char* name) = 0;

	virtual void Update(float deltaTime) = 0;

	virtual IAllocator& GetAllocator() const = 0;

	virtual FileSystem* GetFileSystem() const = 0;
	virtual InputSystem* GetInputSystem() const = 0;
	virtual ResourceManagement* GetResourceManager() const = 0;
};


}