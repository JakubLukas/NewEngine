#pragma once

#include "../widget_base.h"
#include "core/world/world.h"


namespace Veng
{

class Engine;

namespace Editor
{

class WorldsWidget : public WidgetBase
{
public:
	~WorldsWidget() override;
	void Init(Engine& engine, EditorInterface& editor) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "Worlds"; };

private:
	Engine* m_engine = nullptr;
	worldId m_selected;
};

}

}