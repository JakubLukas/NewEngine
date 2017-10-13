#include "engine.h"

#include "input/input_system.h"


namespace Veng
{


class EngineImpl : Engine
{
public:
	EngineImpl(IAllocator& allocator)
		: m_allocator(allocator)
	{
		m_inputSystem = InputSystem::Create(m_allocator);
	}

	~EngineImpl()
	{
		InputSystem::Destroy(m_inputSystem, m_allocator);
	}


	void Update(float deltaTime) override
	{
		m_inputSystem->Update(deltaTime);
	}


	InputSystem* GetInputSystem() const override
	{
		return m_inputSystem;
	}

private:
	InputSystem* m_inputSystem;
	IAllocator& m_allocator;
};


Engine* Engine::Create(IAllocator& allocator)
{
	void* mem = allocator.Allocate(sizeof(EngineImpl));
	return (Engine*)(new (NewPlaceholder(), mem) EngineImpl(allocator));
}


void Engine::Destroy(Engine* engine, IAllocator& allocator)
{
	EngineImpl* p = (EngineImpl*)engine;
	p->~EngineImpl();
	allocator.Deallocate(p);
}


}