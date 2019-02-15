#include "renderer_widget.h"

#include "../widget_register.h"
#include "core/iallocator.h"
#include "core/engine.h"
#include "renderer/pipeline.h"
#include "renderer/renderer.h"
#include "core/math/math.h"
#include "core/math/matrix.h"


namespace Veng
{


namespace Editor
{


RendererWidget::RendererWidget(IAllocator& allocator)
	: m_allocator(allocator)
{

}

RendererWidget::~RendererWidget()
{}


void RendererWidget::Init(Engine& engine)
{
	m_engine = &engine;
	m_renderer = static_cast<RenderSystem*>(engine.GetSystem("renderer"));
	m_pipeline = Pipeline::Create(m_allocator, *m_engine, *m_renderer);
	m_pipeline->Load(Path("pipelines/main.pipeline"));

	for (size_t i = 0; i < /*m_engine->GetWorldCount()*/1; ++i)
	{
		World& world = m_engine->GetWorlds()[i];
		RenderScene* renderScene = static_cast<RenderScene*>(m_renderer->GetScene(world.GetId()));
		m_camera = world.CreateEntity();
		Transform& camTrans = world.GetEntityTransform(m_camera);
		camTrans.position = Vector3(0, 0, -35);
		Camera cam
		{
			Camera::Type::Perspective,
			800.0f,
			600.0f,
			0.1f,
			1000.0f,
			60.0_deg,
		};
		renderScene->AddComponent(RenderScene::GetComponentHandle(RenderScene::Component::Camera), m_camera);
		renderScene->SetComponentData(RenderScene::GetComponentHandle(RenderScene::Component::Camera), m_camera, &cam);
		renderScene->SetMainCamera(m_camera);
	}
}


void RendererWidget::Deinit()
{
	Pipeline::Destroy(m_pipeline);
}


void RendererWidget::Update(EventQueue& queue)
{
	for (size_t i = 0; i < queue.GetPullEventsSize(); ++i)
	{
		const Event* event = queue.PullEvents()[i];
		if (event->type == EventType::SelectCamera)
		{
			const EventSelectCamera* eventCamera = (EventSelectCamera*)event;
			m_camera = eventCamera->camera;
		}
	}
}


void RendererWidget::RenderInternal(EventQueue& queue)
{
	if (nullptr == m_renderer)
	{
		ImGui::Text("No renderer selected");
		return;
	}

	ImVec2 windowSize = ImGui::GetContentRegionAvail();
	if (windowSize != m_size)
	{
		m_size = windowSize;
		OnResize();
		m_changedSize = true;
	}
	else
	{
		m_changedSize = false;
	}

	m_pipeline->Render();
	ImGui::Image(m_pipeline->GetMainFrameBuffer(), windowSize);
}


void RendererWidget::OnResize()
{
	m_renderer->Resize((i32)m_size.x, (i32)m_size.y);
	RenderScene* renderScene = static_cast<RenderScene*>(m_renderer->GetScene(worldId(0)));///////////////////////////////////////
	Camera cam;
	renderScene->GetComponentData(componentHandle(1), m_camera, &cam);
	cam.screenWidth = m_size.x;
	cam.screenHeight = m_size.y;
	renderScene->SetComponentData(componentHandle(1), m_camera, &cam);
}


REGISTER_WIDGET(renderer)
{
	return NEW_OBJECT(allocator, RendererWidget)(allocator);
}


}

}