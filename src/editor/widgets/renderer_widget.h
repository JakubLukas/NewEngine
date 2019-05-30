#pragma once

#include "../widget_base.h"

#include "../external/imgui/imgui.h"


namespace Veng
{

class IAllocator;
class Engine;
class RenderSystem;
class Pipeline;


namespace Editor
{

class RendererWidget : public WidgetBase
{
public:
	explicit RendererWidget(IAllocator& allocator);
	~RendererWidget() override;
	void Init(Engine& engine, EditorInterface& editor) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "Renderer"; };

private:
	void OnResize();

private:
	IAllocator& m_allocator;
	Engine* m_engine = nullptr;
	RenderSystem* m_renderer = nullptr;
	Pipeline* m_pipeline = nullptr;

	ImVec2 m_size = { 0, 0 };
	bool m_changedSize = false;
	bool m_focused = false;

	Entity m_camera = INVALID_ENTITY;
};

}

}