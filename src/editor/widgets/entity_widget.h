#pragma once

#include "../widget_base.h"
#include "core/entity.h"


namespace Veng
{

class Engine;
class World;


namespace Editor
{

class EntityWidget : public WidgetBase
{
public:
	void Init(Engine& engine, EditorInterface& editor) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;
	void Render(EventQueue& queue) override;
	const char* GetName() const override { return "Entity"; };

private:
	Engine* m_engine = nullptr;
	EditorInterface* m_editorInterface;
	World* m_world = nullptr;
	Entity m_entity = INVALID_ENTITY;
};

}

}