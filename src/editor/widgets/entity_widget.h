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
	void Init(IAllocator& allocator, Engine& engine) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "Entity"; };

private:
	Engine* m_engine = nullptr;
	World* m_world = nullptr;
	Entity m_entity = INVALID_ENTITY;
};

}

}