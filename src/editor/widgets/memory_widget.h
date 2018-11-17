#pragma once

#include "../widget_base.h"


namespace Veng
{


namespace Editor
{


class MemoryWidget : public WidgetBase
{
protected:
	void RenderInternal() override;
	const char* GetName() const override { return "MemoryManager"; };
};


}


}