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
	~MemoryWidget() override;
	void Init(IAllocator& allocator, Engine& engine) override;
	void Deinit() override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "MemoryManager"; };

private:
	IAllocator* m_allocator = nullptr;
	const IAllocator* m_selected = nullptr;
	MemoryWidgetData* m_data = nullptr;
};


}


}