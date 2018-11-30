#pragma once

#include "../widget_base.h"

#include <bgfx/bgfx.h>
#include "../external/imgui/imgui.h"
#include "main/app.h"


namespace Veng
{

class Engine;
class RenderSystem;


namespace Editor
{

class RendererWidget : public WidgetBase
{
public:
	~RendererWidget() override;
	void Init(IAllocator& allocator, Engine& engine) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "Renderer"; };

private:
	void OnResize();

private:
	RenderSystem* m_renderer = nullptr;

	windowHandle m_hwnd = INVALID_WINDOW_HANDLE;
	bgfx::ViewId m_viewId = -1;
	bgfx::FrameBufferHandle m_fbh = BGFX_INVALID_HANDLE;
	ImVec2 m_size = { 20, 20 };
	bool m_changedSize = false;

	Engine* m_engine = nullptr;
	Entity m_camera = INVALID_ENTITY;
};

}

}