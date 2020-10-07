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
	void Render(EventQueue& queue) override;

	const char* GetName() const override { return "Entities"; };
	bool HasMenuBar() const override { return true; }

private:
	Engine* m_engine = nullptr;
	World* m_world = nullptr;
	Entity m_selected = INVALID_ENTITY;
};

}

}