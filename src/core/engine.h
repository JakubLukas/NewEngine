#pragma once

#include "allocators.h"


namespace Veng
{

class InputSystem;


class Engine
{
public:
	static Engine* Create(IAllocator& allocator);
	static void Destroy(Engine* engine, IAllocator& allocator);

	virtual void Update(float deltaTime) = 0;

	virtual InputSystem* GetInputSystem() const = 0;
};


}