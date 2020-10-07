#pragma once

#include "../widget_base.h"

#include "../external/imgui/imgui.h"


namespace Veng
{

class Allocator;
class Engine;
class RenderSystem;
class Pipeline;


namespace Editor
{

class RendererWidget : public WidgetBase
{
public:
	explicit RendererWidget(Allocator& allocator);
	~RendererWidget() override;
	void Init(Engine& engine, EditorInterface& editor) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void Render(EventQueue& queue) override;
	const char* GetName() const override { return "Renderer"; };

private:
	void OnResize();

private:
	Allocator& m_allocator;
	Engine* m_engine = nullptr;
	RenderSystem* m_renderer = nullptr;
	Pipeline* m_pipeline = nullptr;
	worldId m_world = worldId(0);

	ImVec2 m_size = { 0, 0 };

	Entity m_camera = INVALID_ENTITY;
	bool m_cameraControl = false;
};

}

}