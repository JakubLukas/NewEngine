#pragma once

#include "../widget_base.h"
#include "core/containers/hash_map.h"


namespace Veng
{

class Allocator;


namespace Editor
{

struct MemoryWidgetData;


class MemoryWidget : public WidgetBase
{
public:
	MemoryWidget(Allocator& allocator);
	~MemoryWidget() override;
	void Init(Engine& engine, EditorInterface& editor) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;
	void Render(EventQueue& queue) override;
	const char* GetName() const override { return "Memory viewer"; };

private:
	Allocator& m_allocator;
	const Allocator* m_selected = nullptr;
	MemoryWidgetData* m_data = nullptr;
};


}


}