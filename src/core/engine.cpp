#include "engine.h"

#include "file/file_system.h"
#include "input/input_system.h"
#include "resource/resource_manager.h"
#include "resource/resource_management.h"
#include "string.h"
#include "file/blob.h"
#include "logs.h"


namespace Veng
{

static const u32 SERIALIZE_MAGIC = ('!' << 24) + ('N' << 16) + ('e' << 8) + ('N');
enum class Version : u32
{
	First = 0,

	Latest
};


class EngineImpl : public Engine
{
public:
	EngineImpl(Allocator& allocator)
		: m_allocator(allocator)
		, m_systems(m_allocator)
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


	bool Serialize(OutputBlob& serializer) const override
	{
		serializer.Write(SERIALIZE_MAGIC);
		serializer.Write(Version::Latest);

		serializer.Write((u32)m_worlds.GetSize());
		for (const auto& world : m_worlds)
		{
			serializer.Write(world.GetId());
			world.Serialize(serializer);
		}

		serializer.Write((u32)m_systems.GetSize());
		for (const System* system : m_systems)
		{
			serializer.Write(system->GetName());
			serializer.Write(system->GetVersion());
			size_t system_pos = serializer.GetPosition();
			serializer.Write((u64)system_pos);
			system->Serialize(serializer);
			serializer.WriteAtPos((u64)(serializer.GetPosition() - system_pos), system_pos);
		}
		return true;
	}

	bool Deserialize(InputBlob& serializer) override
	{
		u32 magic;
		serializer.Read(magic);
		if (magic != SERIALIZE_MAGIC) return false;
		Version version;
		serializer.Read(version);
		if (version != Version::Latest) return false;

		u32 count;
		serializer.Read(count);
		for (u32 i = 0; i < count; ++i)
		{
			worldId wId;
			serializer.Read(wId);
			World& world = m_worlds.EmplaceBack(m_allocator, wId);
			world.Deserialize(serializer);
		}

		serializer.Read(count);
		for (u32 i = 0; i < count; ++i)
		{
			String systemName(m_allocator);
			serializer.Read(systemName);
			u32 version;
			serializer.Read(version);
			u64 bytesSerialized;
			serializer.Read(bytesSerialized);

			System* system = GetSystem(systemName.Cstr());
			if (system == nullptr)
			{
				Log(LogType::Warning, "During deserialization, \"%s\" system was not found in registered systems. Skipping system's serialized data.", systemName.Cstr());
				serializer.Skip(bytesSerialized);
			}
			else if (version != system->GetVersion())
			{
				Log(LogType::Warning, "During deserialization, \"%s\" system version mismatch. Skipping system's serialized data.", systemName.Cstr());
				serializer.Skip(bytesSerialized);
			}
			else
			{
				system->Deserialize(serializer);
			}
		}
		return true;
	}


	worldId AddWorld() override
	{
		World& world = m_worlds.EmplaceBack(m_allocator, (worldId)m_worlds.GetSize());

		for (System* system : m_systems)
			system->OnWorldAdded(world.GetId());

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
		{
			for (System* system : m_systems)
				system->OnWorldRemoved(id);

			m_worlds.Erase(idx);
		}
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

	size_t GetWorldCount() const override
	{
		return m_worlds.GetSize();
	}

	World* GetWorlds() override
	{
		return m_worlds.Begin();
	}


	bool AddSystem(System* system) override
	{
		m_systems.PushBack(system);
		return true;
	}

	bool RemoveSystem(const char* name) override
	{
		for (unsigned i = 0; i < m_systems.GetSize(); ++i)
		{
			if (string::Equal(m_systems[i]->GetName(), name))
			{
				m_systems.Erase(i);
				return true;
			}
		}
		return false;
	}

	System* GetSystem(const char* name) const override
	{
		for (unsigned i = 0; i < m_systems.GetSize(); ++i)
		{
			if (string::Equal(m_systems[i]->GetName(), name))
			{
				return m_systems[i];
			}
		}

		return nullptr;
	}

	size_t GetSystemCount() const override
	{
		return m_systems.GetSize();
	}

	System* const* GetSystems() const override
	{
		return m_systems.Begin();
	}


	bool AddResourceManager(ResourceManager& manager) override
	{
		return m_resourceManager->RegisterManager(manager.GetType(), &manager);
	}

	bool RemoveResourceManager(ResourceType type) override
	{
		return m_resourceManager->UnregisterManager(type);
	}

	ResourceManager* GetResourceManager(ResourceType type) const override
	{
		return m_resourceManager->GetManager(type);
	}

	ResourceManagement* GetResourceManagement() const override
	{
		return m_resourceManager;
	}


	void Update(float deltaTime) override
	{
		for (System* plugin : m_systems)
		{
			plugin->Update(deltaTime);
		}
		m_fileSystem->Update(deltaTime);
		m_inputSystem->Update(deltaTime);
	}


	Allocator& GetAllocator() const override
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

private:
	Allocator& m_allocator;
	FileSystem* m_fileSystem;
	InputSystem* m_inputSystem;
	ResourceManagement* m_resourceManager;
	Array<System*> m_systems;
	Array<World> m_worlds;
};


Engine* Engine::Create(Allocator& allocator)
{
	return NEW_OBJECT(allocator, EngineImpl)(allocator);
}


void Engine::Destroy(Engine* engine, Allocator& allocator)
{
	EngineImpl* ptr = static_cast<EngineImpl*>(engine);
	DELETE_OBJECT(allocator, ptr);
}


}
