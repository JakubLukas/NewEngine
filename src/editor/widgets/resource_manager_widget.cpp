#include "resource_manager_widget.h"

#include "../external/imgui/imgui.h"
#include "core/engine.h"


namespace Veng
{


namespace Editor
{


ResourceManagerWidget::~ResourceManagerWidget()
{}


void ResourceManagerWidget::Init(IAllocator& allocator, Engine& engine)
{
	m_manager = engine.GetResourceManagement();
}


void ResourceManagerWidget::Deinit()
{}


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