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

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "Entities"; };

private:
	Engine* m_engine = nullptr;
	World* m_world = nullptr;
	Entity m_selected = INVALID_ENTITY;
};

}

}