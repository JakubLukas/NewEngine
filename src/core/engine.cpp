#include "engine.h"

#include "file/file_system.h"
#include "input/input_system.h"
#include "resource/resource_management.h"
#include "string.h"


namespace Veng
{


class EngineImpl : public Engine
{
public:
	EngineImpl(IAllocator& allocator)
		: m_allocator(allocator)
		, m_plugins(m_allocator)
		, m_worlds(m_allocator)
	{
		m_fileSystem = FileSystem::Create(m_allocator);
		m_inputSystem = InputSystem::Create(m_allocator);
		m_resourceManager = ResourceManagement::Create(m_allocator);
	}

	~EngineImpl()
	{
		ResourceManagement::Destroy(m_resourceManager, m_allocator);
		InputSystem::Destroy(m_inputSystem, m_allocator);
		FileSystem::Destroy(m_fileSystem, m_allocator);
	}


	void SetPlatformData(const PlatformData& data) override
	{
		m_platformData = data;
	}

	const PlatformData& GetPlatformData() const override
	{
		return m_platformData;
	}


	worldId AddWorld() override
	{
		World& world = m_worlds.EmplaceBack(m_allocator, (worldId)m_worlds.GetSize());
		return world.GetId();
	}

	void RemoveWorld(worldId id) override
	{
		size_t idx = m_worlds.GetSize();
		for (size_t i = 0; i < m_worlds.GetSize(); ++i)
		{
			if (m_worlds[i].GetId() == id)
				idx = i;
		}

		if (idx != m_worlds.GetSize())
			m_worlds.Erase(idx);
		else
			ASSERT2(false, "World with given id doesn't exist");
	}

	World* GetWorld(worldId id) override
	{
		for (World& world : m_worlds)
		{
			if (world.GetId() == id)
				return &world;
		}
		return nullptr;
	}


	bool AddPlugin(IPlugin* plugin) override
	{
		m_plugins.Push(plugin);
		return true;
	}


	void RemovePlugin(const char* name) override
	{
		for (unsigned i = 0; i < m_plugins.GetSize(); ++i)
		{
			if (string::Equal(m_plugins[i]->GetName(), name))
			{
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
		m_fileSystem->Update(deltaTime);
		m_inputSystem->Update(deltaTime);
	}


	IAllocator& GetAllocator() const override
	{
		return m_allocator;
	}


	FileSystem* GetFileSystem() const override
	{
		return m_fileSystem;
	}


	InputSystem* GetInputSystem() const override
	{
		return m_inputSystem;
	}


	ResourceManagement* GetResourceManagement() const override
	{
		return m_resourceManager;
	}

private:
	PlatformData m_platformData;
	IAllocator& m_allocator;
	FileSystem* m_fileSystem;
	InputSystem* m_inputSystem;
	ResourceManagement* m_resourceManager;
	Array<IPlugin*> m_plugins;
	Array<World> m_worlds;
};


Engine* Engine::Create(IAllocator& allocator)
{
	return NEW_OBJECT(allocator, EngineImpl)(allocator);
}


void Engine::Destroy(Engine* engine, IAllocator& allocator)
{
	EngineImpl* ptr = static_cast<EngineImpl*>(engine);
	DELETE_OBJECT(allocator, ptr);
}


}