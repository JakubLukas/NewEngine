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
	void SetEngine(Engine* engine);
	void SetEntity(Entity entity);

protected:
	void RenderInternal() override;
	const char* GetName() const override { return "Entity"; };

private:
	Engine* m_engine = nullptr;
	Entity m_entity = INVALID_ENTITY;
};

}

}