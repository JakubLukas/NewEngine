#pragma once

#include "allocators.h"
#include "iplugin.h"


namespace Veng
{

class InputSystem;


class Engine
{
public:
	static Engine* Create(IAllocator& allocator);
	static void Destroy(Engine* engine, IAllocator& allocator);

	virtual bool AddPlugin(IPlugin* plugin) = 0;
	virtual void RemovePlugin(const char* name) = 0;

	virtual void Update(float deltaTime) = 0;

	virtual IAllocator& GetAllocator() const = 0;
	virtual InputSystem* GetInputSystem() const = 0;
};


}