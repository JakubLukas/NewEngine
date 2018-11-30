#include "renderer_widget.h"

#include "../widget_register.h"
#include "core/engine.h"
#include "renderer/renderer.h"
#include "core/math/math.h"
#include "core/math/matrix.h"


namespace Veng
{


namespace Editor
{


RendererWidget::~RendererWidget()
{}


void RendererWidget::Init(IAllocator& allocator, Engine& engine)
{
	m_engine = &engine;
	m_renderer = static_cast<RenderSystem*>(engine.GetSystem("renderer"));
	m_viewId = 1;

	m_fbh = bgfx::createFrameBuffer(0, 0, bgfx::TextureFormat::Enum::RGB8);
	bgfx::setViewFrameBuffer(m_viewId, m_fbh);
	bgfx::setViewRect(m_viewId, 0, 0, uint16_t(m_size.x), uint16_t(m_size.y));

	bgfx::setViewClear(m_viewId
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0x803030ff
		, 1.0f
		, 0
	);

	RenderScene* renderScene = static_cast<RenderScene*>(m_renderer->GetScene());
	for (size_t i = 0; i < m_engine->GetWorldCount(); ++i)
	{
		World& world = m_engine->GetWorlds()[i];
		m_camera = world.CreateEntity();
		Transform& camTrans = world.GetEntityTransform(m_camera);
		camTrans.position = Vector3(0, 0, 35);
		renderScene->AddCameraComponent(m_camera, world.GetId(), 60.0_deg, 0.001f, 100.0f);
	}
}


void RendererWidget::Deinit()
{
	bgfx::destroy(m_fbh);
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

	ImGui::Image((void*)&m_fbh, windowSize);
}


void RendererWidget::OnResize()
{
	bgfx::destroy(m_fbh);
	m_fbh = bgfx::createFrameBuffer(uint16_t(m_size.x), uint16_t(m_size.y), bgfx::TextureFormat::Enum::RGB8);
	bgfx::setViewFrameBuffer(m_viewId, m_fbh);
	bgfx::setViewRect(m_viewId, 0, 0, uint16_t(m_size.x), uint16_t(m_size.y));

	m_renderer->Resize((i32)m_size.x, (i32)m_size.y);
	RenderScene* renderScene = static_cast<RenderScene*>(m_renderer->GetScene());
	renderScene->SetCameraScreenSize(m_camera, m_size.x, m_size.y);
}


REGISTER_WIDGET(renderer)
{
	return NEW_OBJECT(allocator, RendererWidget)();
}


}

}