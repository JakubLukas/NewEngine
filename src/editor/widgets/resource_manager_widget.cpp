#include "resource_manager_widget.h"

#include "../external/imgui/imgui.h"


namespace Veng
{


namespace Editor
{


void ResourceManagerWidget::SetResourceManager(ResourceManagement& manager)
{
}


void ResourceManagerWidget::RenderInternal()
{
	if (nullptr == m_manager)
	{
		ImGui::Text("No resource manager selected");
		return;
	}

	ImGui::Text("Resource manager ok");
}


}


}