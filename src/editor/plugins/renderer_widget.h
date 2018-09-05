#pragma once

#include "../widget_base.h"

#include <bgfx/bgfx.h>
#include "../external/imgui/imgui.h"
#include "main/app.h"


namespace Veng
{

class RenderSystem;


namespace Editor
{

class RendererWidget : public WidgetBase
{
public:
	void Init(bgfx::ViewId viewId);
	void Deinit();
	void SetRenderSystem(RenderSystem* system);

protected:
	void RenderInternal() override;
	const char* GetName() const override { return "Renderer"; };

private:
	RenderSystem* m_renderer = nullptr;

	windowHandle m_hwnd = INVALID_WINDOW_HANDLE;
	bgfx::ViewId m_viewId = -1;
	bgfx::FrameBufferHandle m_fbh = BGFX_INVALID_HANDLE;
	ImVec2 m_size = { 20, 20 };
};

}

}