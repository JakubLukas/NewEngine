#pragma once

#include "../widget_base.h"
#include "core/resource/resource.h"


namespace Veng
{

class IAllocator;
class Engine;
class ResourceManagement;


namespace Editor
{


class ModelWidget : public WidgetBase
{
public:
	ModelWidget(IAllocator& allocator);
	~ModelWidget() override;
	void Init(Engine& engine) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "ModelEditor"; };

private:
	IAllocator& m_allocator;
	ResourceManagement* m_manager;
	resourceHandle m_model = INVALID_RESOURCE_HANDLE;
};


}


}