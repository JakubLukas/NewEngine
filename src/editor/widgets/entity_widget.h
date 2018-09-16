#pragma once

#include "../widget_base.h"


namespace Veng
{

class Engine;


namespace Editor
{

class EntityWidget : public WidgetBase
{
public:
	void SetEngine(Engine* engine);

protected:
	void RenderInternal() override;
	const char* GetName() const override { return "Entity"; };

private:
	Engine* m_engine = nullptr;
};

}

}