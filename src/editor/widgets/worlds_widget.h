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
	void Init(IAllocator& allocator, Engine& engine) override;
	void Deinit() override;

	worldId GetSelected() const;//TODO: event

protected:
	void RenderInternal() override;
	const char* GetName() const override { return "Worlds"; };

private:
	Engine* m_engine = nullptr;
	worldId m_selected;
};

}

}