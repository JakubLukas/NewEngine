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
	void SetResourceManager(ResourceManagement& manager);

protected:
	void RenderInternal() override;
	const char* GetName() const override { return "ResourceManager"; };

private:
	ResourceManagement* m_manager = nullptr;
};


}


}