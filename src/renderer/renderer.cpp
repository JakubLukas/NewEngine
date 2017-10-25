#include "irenderer.h"

#include "core/allocators.h"
#include "core/engine.h"


namespace Veng
{


class RenderSystemImpl : public RenderSystem
{
public:
	RenderSystemImpl(Engine& engine)
		: m_engine(engine)
		, m_allocator(engine.GetAllocator())
	{

	}


	~RenderSystemImpl() override
	{
	
	}


	void Update(float deltaTime) override
	{

	};


	const char* GetName() const override { return "renderer"; }

	Engine& GetEngine() const override { return m_engine; }

private:
	Engine& m_engine;
	IAllocator& m_allocator;
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