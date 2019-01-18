#pragma once

#include "../widget_base.h"
#include "core/resource/resource.h"


namespace Veng
{

class IAllocator;
class Engine;
class ResourceManager;


namespace Editor
{


class MaterialWidget : public WidgetBase
{
public:
	MaterialWidget(IAllocator& allocator);
	~MaterialWidget() override;
	void Init(Engine& engine) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "MaterialEditor"; };

private:
	IAllocator& m_allocator;
	ResourceManager* m_manager;
	resourceHandle m_material = INVALID_RESOURCE_HANDLE;
};


}


}