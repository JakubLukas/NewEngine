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
	void Init(IAllocator& allocator, Engine& engine) override;
	void Deinit() override;

	void SetWorld(World* world);//TODO: event
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