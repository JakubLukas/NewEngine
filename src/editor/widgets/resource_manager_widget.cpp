#include "resource_manager_widget.h"

#include "../widget_register.h"
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


void ResourceManagerWidget::Update(EventQueue& queue)
{}


void ResourceManagerWidget::RenderInternal(EventQueue& queue)
{
	if (nullptr == m_manager)
	{
		ImGui::Text("No resource manager selected");
		return;
	}

	ImGui::Text("Resource manager ok");
}


REGISTER_WIDGET(resource_manager)
{
	return NEW_OBJECT(allocator, ResourceManagerWidget)();
}


}


}