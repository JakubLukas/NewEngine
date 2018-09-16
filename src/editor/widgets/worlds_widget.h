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
	void SetEngine(Engine* engine);
	worldId GetSelected() const;

protected:
	void RenderInternal() override;
	const char* GetName() const override { return "Worlds"; };

private:
	Engine* m_engine = nullptr;
	worldId m_selected;
};

}

}