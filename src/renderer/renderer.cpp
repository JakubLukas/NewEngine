#include "irenderer.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "core/associative_array.h"


namespace Veng
{


class RenderSystemImpl : public RenderSystem
{
public:
	RenderSystemImpl(Engine& engine)
		: m_engine(engine)
		, m_allocator(HeapAllocator(engine.GetAllocator()))
		, m_meshes(m_allocator)
	{

	}


	~RenderSystemImpl() override
	{
	
	}


	void Update(float deltaTime) override
	{

	};


	const char* GetName() const override { return "renderer"; }


	void AddMeshComponent(Entity entity, worldId world) override
	{
		m_meshes.Insert(entity, Mesh());
	}

	void RemoveMeshComponent(Entity entity, worldId world) override
	{
		m_meshes.Erase(entity);
	}

	bool HasMeshComponent(Entity entity, worldId world) override
	{
		Mesh* mesh;
		return m_meshes.Find(entity, mesh);
	}


	Engine& GetEngine() const override { return m_engine; }

private:
	Engine& m_engine;
	IAllocator& m_allocator;
	AssociativeArray<Entity, Mesh> m_meshes;
};


RenderSystem* RenderSystem::Create(Engine& engine)
{
	return (RenderSystem*)new (NewPlaceholder(), engine.GetAllocator()) RenderSystemImpl(engine);
}

void RenderSystem::Destroy(RenderSystem* system)
{
	RenderSystemImpl* p = (RenderSystemImpl*)system;
	IAllocator& allocator = system->GetEngine().GetAllocator();
	p->~RenderSystemImpl();
	allocator.Deallocate(p);
}

}