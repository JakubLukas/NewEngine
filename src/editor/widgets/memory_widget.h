#pragma once

#include "../widget_base.h"
#include "core/containers/hash_map.h"


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
	~MemoryWidget() override;
	void Init(Engine& engine) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "MemoryManager"; };

private:
	IAllocator& m_allocator;
	const IAllocator* m_selected = nullptr;
	MemoryWidgetData* m_data = nullptr;
};


}


}