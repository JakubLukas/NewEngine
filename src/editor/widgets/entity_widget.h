#pragma once

#include "../widget_base.h"
#include "core/entity.h"


namespace Veng
{

class Engine;


namespace Editor
{

class EntityWidget : public WidgetBase
{
public:
	void Init(IAllocator& allocator, Engine& engine) override;
	void Deinit() override;

	void SetEntity(Entity entity);//TODO: event

protected:
	void RenderInternal() override;
	const char* GetName() const override { return "Entity"; };

private:
	Engine* m_engine = nullptr;
	Entity m_entity = INVALID_ENTITY;
};

}

}