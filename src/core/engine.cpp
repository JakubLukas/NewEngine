#include "engine.h"

#include "input/input_system.h"
#include "string.h"


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
		m_plugins.Push(plugin);
		return true;
	}


	void RemovePlugin(const char* name) override
	{
		for (unsigned i = 0; i < m_plugins.Size(); ++i)
		{
			if (StrEqual(m_plugins[i]->GetName(), name))
			{
				m_plugins[i]->~IPlugin();
				m_plugins.Erase(i);
				return;
			}
		}
	}


	void Update(float deltaTime) override
	{
		for (IPlugin* plugin : m_plugins)
		{
			plugin->Update(deltaTime);
		}
		m_inputSystem->Update(deltaTime);
	}


	IAllocator& GetAllocator() const override
	{
		return m_allocator;
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