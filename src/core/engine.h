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

	virtual InputSystem* GetInputSystem() const = 0;
};


}