#pragma once

#include "allocators.h"
#include "plugin.h"


namespace Veng
{

class InputSystem;


class Engine
{
public:
	static Engine* Create(IAllocator& allocator);
	static void Destroy(Engine* engine, IAllocator& allocator);

	virtual bool AddPlugin(IPlugin* plugin) = 0;
	virtual void RemovePlugin(IPlugin* plugin) = 0;

	virtual void Update(float deltaTime) = 0;

	virtual InputSystem* GetInputSystem() const = 0;
};


}