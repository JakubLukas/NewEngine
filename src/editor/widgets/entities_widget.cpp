#include "entities_widget.h"

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"
#include "core/world/world.h"

namespace Veng
{

namespace Editor
{


void EntitiesWidget::Init(IAllocator& allocator, Engine& engine)
{}


void EntitiesWidget::Deinit()
{}


void EntitiesWidget::SetWorld(World* world)
{
	m_world = world;
}


Entity EntitiesWidget::GetSelected() const
{
	return m_selected;
}


void EntitiesWidget::RenderInternal()
{
	if (nullptr == m_world)
	{
		ImGui::Text("No world selected");
		return;
	}

	ImGui::Text("WorldId: %i", m_world->GetId());
	ImGui::Separator();

	ImGui::PushItemWidth(-1);
	if(!ImGui::ListBoxHeader("##empty", ImGui::GetContentRegionAvail()))
	{
		ImGui::Text("Error: ListBoxHeader wasn't created");
		ImGui::PopItemWidth();
		return;
	}

	World::EntityIterator& entityIter = m_world->GetEntities();
	Entity entity;
	for(int i = 0; entityIter.GetNext(entity); ++i)
	{
		const bool item_selected = (entity == m_selected);

		char item_text[64] = { '\0' };
		ImFormatString(item_text, 64, "Entity %i (id:%i)", i, (u64)entity);

		ImGui::PushID(i);
		if(ImGui::Selectable(item_text, item_selected))
		{
			m_selected = entity;
		}
		if(item_selected)
			ImGui::SetItemDefaultFocus();
		ImGui::PopID();
	}

	ImGui::ListBoxFooter();
	ImGui::PopItemWidth();
}


}

}