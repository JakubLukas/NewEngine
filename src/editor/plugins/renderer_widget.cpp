#include "renderer_widget.h"

#include "renderer/irenderer.h"


namespace Veng
{

namespace Editor
{


void RendererWidget::Init(bgfx::ViewId viewId)
{
	m_viewId = viewId;

	m_fbh = bgfx::createFrameBuffer(0, 0, bgfx::TextureFormat::Enum::RGB8);
	bgfx::setViewFrameBuffer(m_viewId, m_fbh);
	bgfx::setViewRect(m_viewId, 0, 0, uint16_t(m_size.x), uint16_t(m_size.y));

	bgfx::setViewClear(m_viewId
		, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH
		, 0x803030ff
		, 1.0f
		, 0
	);
}

void RendererWidget::Deinit()
{
	bgfx::destroy(m_fbh);
}


void RendererWidget::SetRenderSystem(RenderSystem* system)
{
	m_renderer = system;
}


void RendererWidget::RenderInternal()
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

		bgfx::destroy(m_fbh);
		m_fbh = bgfx::createFrameBuffer(uint16_t(windowSize.x), uint16_t(windowSize.y), bgfx::TextureFormat::Enum::RGB8);
		bgfx::setViewFrameBuffer(m_viewId, m_fbh);
		bgfx::setViewRect(m_viewId, 0, 0, uint16_t(windowSize.x), uint16_t(windowSize.y));

		m_renderer->Resize((i32)m_size.x, (i32)m_size.y);
	}

	ImGui::Image((void*)&m_fbh, windowSize);
}


}

}