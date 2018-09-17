#include "worlds_widget.h"

#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"
#include "core/engine.h"
#include "core/asserts.h"


namespace Veng
{

namespace Editor
{

void WorldsWidget::SetEngine(Engine* engine)
{
	m_engine = engine;
}


worldId WorldsWidget::GetSelected() const
{
	return m_selected;
}


void WorldsWidget::RenderInternal()
{
	size_t worldsCount = m_engine->GetWorldCount();
	const World* worlds = m_engine->GetWorlds();

	ImGui::PushItemWidth(-1);
	if (!ImGui::ListBoxHeader("##empty"))
	{
		ImGui::Text("Error: ListBoxHeader wasn't created");
		ImGui::PopItemWidth();
		return;
	}

	// Assume all items have even height (= 1 line of text). If you need items of different or variable sizes you can create a custom version of ListBox() in your code without using the clipper.
	ImGuiListClipper clipper((int)worldsCount, ImGui::GetTextLineHeightWithSpacing()); // We know exactly our line height here so we pass it as a minor optimization, but generally you don't need to.
	while (clipper.Step())
	{
		for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
		{
			const bool item_selected = (worlds[i].GetId() == m_selected);
			char item_text[64] = {'\0'};
			ImFormatString(item_text, 64, "World %i (id:%i)", i, worlds[i].GetId());

			ImGui::PushID(i);
			if (ImGui::Selectable(item_text, item_selected))
			{
				m_selected = worlds[i].GetId();
			}
			if (item_selected)
				ImGui::SetItemDefaultFocus();
			ImGui::PopID();
		}
	}

	ImGui::ListBoxFooter();
	ImGui::PopItemWidth();
}


}

}