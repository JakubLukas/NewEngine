#pragma once

#include "../widget_base.h"
#include "core/resource/resource.h"


namespace Veng
{

class Allocator;
class Engine;
class ResourceManagement;


namespace Editor
{


class ModelWidget : public WidgetBase
{
public:
	ModelWidget(Allocator& allocator);
	~ModelWidget() override;
	void Init(Engine& engine, EditorInterface& editor) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;
	void Render(EventQueue& queue) override;
	const char* GetName() const override { return "ModelEditor"; };

private:
	Allocator& m_allocator;
	ResourceManagement* m_manager;
	resourceHandle m_modelHandle = INVALID_RESOURCE_HANDLE;
};


}


}