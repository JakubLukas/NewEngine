#include "pipeline.h"

#include "core/allocators.h"
#include "core/engine.h"
#include "renderer.h"

#include "core/math/matrix.h"


namespace Veng
{


class PipelineImpl : public Pipeline
{
public:
	PipelineImpl(IAllocator& allocator, Engine& engine, RenderSystem& renderer)
		: m_sourceAllocator(allocator)
		, m_allocator(allocator)
		, m_engine(engine)
		, m_renderer(renderer)
	{
		m_allocator.SetDebugName("Pipeline");
	}

	~PipelineImpl()
	{}

	void Load() override
	{}

	void Render() override
	{
		const RenderScene* scene = (RenderScene*)m_renderer.GetScene();

		worldId worldHandle = (worldId)0;
		World* world = m_engine.GetWorld(worldHandle);//////////////

		const RenderScene::CameraItem* cameraItem = scene->GetDefaultCamera(worldHandle);
		m_renderer.SetCamera(cameraItem->entity);

		const RenderScene::ModelItem* modelItems = scene->GetModels(worldHandle);
		m_renderer.RenderModels(*world, modelItems, scene->GetModelsCount(worldHandle));

		m_renderer.Frame();
	}

	IAllocator& GetSourceAllocator() { return m_sourceAllocator; }

private:
	IAllocator& m_sourceAllocator;
	ProxyAllocator m_allocator;
	Engine& m_engine;
	RenderSystem& m_renderer;

	//FramebufferHandle 
};


Pipeline* Pipeline::Create(IAllocator& allocator, Engine& engine, RenderSystem& renderer)
{
	return NEW_OBJECT(allocator, PipelineImpl)(allocator, engine, renderer);
}

void Pipeline::Destroy(Pipeline* pipeline)
{
	PipelineImpl* pipe = static_cast<PipelineImpl*>(pipeline);
	IAllocator& allocator = pipe->GetSourceAllocator();
	DELETE_OBJECT(allocator, pipe);
}


}