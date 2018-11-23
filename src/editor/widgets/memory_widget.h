#pragma once

#include "../widget_base.h"


namespace Veng
{

class IAllocator;


namespace Editor
{

struct MemoryWidgetData;


class MemoryWidget : public WidgetBase
{
public:
	MemoryWidget(IAllocator& allocator);
	~MemoryWidget();

protected:
	void RenderInternal() override;
	const char* GetName() const override { return "MemoryManager"; };

private:
	IAllocator& m_allocator;
	const IAllocator* m_selected = nullptr;
	MemoryWidgetData* m_data;
};


}


}