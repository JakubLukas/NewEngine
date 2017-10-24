#include "engine.h"

#include "input/input_system.h"


namespace Veng
{


class EngineImpl : Engine
{
public:
	EngineImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_plugins(m_allocator)
	{
		m_inputSystem = InputSystem::Create(m_allocator);
	}

	~EngineImpl()
	{
		InputSystem::Destroy(m_inputSystem, m_allocator);
	}


	bool AddPlugin(IPlugin* plugin) override
	{
		return false;
	}


	void RemovePlugin(IPlugin* plugin) override
	{

	}


	void Update(float deltaTime) override
	{
		for (IPlugin* plugin : m_plugins)
		{
			plugin->Update(deltaTime);
		}
		m_inputSystem->Update(deltaTime);
	}


	InputSystem* GetInputSystem() const override
	{
		return m_inputSystem;
	}

private:
	InputSystem* m_inputSystem;
	IAllocator& m_allocator;
	Array<IPlugin*> m_plugins;
};


Engine* Engine::Create(IAllocator& allocator)
{
	return (Engine*)new (NewPlaceholder(), allocator) EngineImpl(allocator);
}


void Engine::Destroy(Engine* engine, IAllocator& allocator)
{
	EngineImpl* p = (EngineImpl*)engine;
	p->~EngineImpl();
	allocator.Deallocate(p);
}


}