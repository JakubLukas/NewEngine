#pragma once

#include "../widget_base.h"


namespace Veng
{

class IAllocator;


namespace Editor
{


class MemoryWidget : public WidgetBase
{
protected:
	void RenderInternal() override;
	const char* GetName() const override { return "MemoryManager"; };

private:
	const IAllocator* m_selected = nullptr;
};


}


}