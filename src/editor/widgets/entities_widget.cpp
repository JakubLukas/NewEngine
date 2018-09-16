#include "entities_widget.h"

#include "../external/imgui/imgui.h"
#include "core/world/world.h"

namespace Veng
{

namespace Editor
{


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

	World::EntityIterator& entityIter = m_world->GetEntities();
	Entity entity;
	for (int i = 0; entityIter.GetNext(entity); ++i)
	{
		ImGui::Text("Entity %i (id:%i) ", i, (u64)entity);
	}


	ImGui::PushItemWidth(-1);
	if(!ImGui::ListBoxHeader("##empty"))
	{
		ImGui::Text("Error: ListBoxHeader wasn't created");
		ImGui::PopItemWidth();
		return;
	}

	// Assume all items have even height (= 1 line of text). If you need items of different or variable sizes you can create a custom version of ListBox() in your code without using the clipper.
	ImGuiListClipper clipper((int)worldsCount, ImGui::GetTextLineHeightWithSpacing()); // We know exactly our line height here so we pass it as a minor optimization, but generally you don't need to.
	while(clipper.Step())
	{
		for(int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const bool item_selected = (worlds[i].GetId() == m_selected);
			char item_text[64] = { '\0' };
			ImFormatString(item_text, 64, "World %i (id:%i)", i, worlds[i].GetId());

			ImGui::PushID(i);
			if(ImGui::Selectable(item_text, item_selected))
			{
				m_selected = worlds[i].GetId();
			}
			if(item_selected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
	}

	ImGui::ListBoxFooter();
	ImGui::PopItemWidth();
}


}

}