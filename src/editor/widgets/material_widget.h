#pragma once

#include "../widget_base.h"
#include "core/resource/resource.h"


namespace Veng
{

class Allocator;
class Engine;
class ResourceManager;
class Pipeline;


namespace Editor
{


class MaterialWidget : public WidgetBase
{
public:
	MaterialWidget(Allocator& allocator);
	~MaterialWidget() override;
	void Init(Engine& engine, EditorInterface& editor) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;
	void Render(EventQueue& queue) override;
	const char* GetName() const override { return "MaterialEditor"; };

private:
	Allocator& m_allocator;
	ResourceManager* m_manager;
	Pipeline* m_pipeline;
	resourceHandle m_material = INVALID_RESOURCE_HANDLE;
};


}


}