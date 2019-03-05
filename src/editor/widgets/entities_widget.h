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
	void Init(Engine& engine, EditorInterface& editor) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

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