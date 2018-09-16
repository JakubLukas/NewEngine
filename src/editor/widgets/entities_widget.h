#pragma once

#include "../widget_base.h"
#include "core/entity.h"


namespace Veng
{

class World;


namespace Editor
{

class EntitiesWidget : public WidgetBase
{
public:
	void SetWorld(World* world);
	Entity GetSelected() const;

protected:
	void RenderInternal() override;
	const char* GetName() const override { return "Entities"; };

private:
	World* m_world = nullptr;
	Entity m_selected = INVALID_ENTITY;
};

}

}