#pragma once

#include "../widget_base.h"


namespace Veng
{

class ResourceManagement;


namespace Editor
{


class ResourceManagerWidget : public WidgetBase
{
public:
	~ResourceManagerWidget() override;
	void Init(IAllocator& allocator, Engine& engine) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "ResourceManager"; };

private:
	ResourceManagement* m_manager = nullptr;
};


}


}