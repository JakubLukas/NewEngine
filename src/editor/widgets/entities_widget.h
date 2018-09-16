#pragma once

#include "../widget_base.h"


namespace Veng
{

class World;


namespace Editor
{

class EntitiesWidget : public WidgetBase
{
public:
	void SetWorld(World* world);

protected:
	void RenderInternal() override;
	const char* GetName() const override { return "Entities"; };

private:
	World* m_world = nullptr;
};

}

}