#include "entities_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "../external/imgui/imgui_internal.h"
#include "core/engine.h"
#include "core/world/world.h"

namespace Veng
{

namespace Editor
{


void EntitiesWidget::Init(IAllocator& allocator, Engine& engine)
{
	m_engine = &engine;
}


void EntitiesWidget::Deinit()
{}


void EntitiesWidget::RenderInternal(EventQueue& queue)
{
	for (size_t i = 0; i < queue.GetPullEventsSize(); ++i)
	{
		const Event* event = queue.PullEvents()[i];
		if (event->type == EventType::SelectWorld)
		{
			const EventSelectWorld* eventWorld = (EventSelectWorld*)event;
			m_world = m_engine->GetWorld(eventWorld->id);
		}
	}

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
			EventSelectEntity event;
			event.entity = m_selected;
			queue.PushEvent(event);
		}
		if(item_selected)
			ImGui::SetItemDefaultFocus();
		ImGui::PopID();
	}

	ImGui::ListBoxFooter();
	ImGui::PopItemWidth();
}


REGISTER_WIDGET(entities)
{
	return NEW_OBJECT(allocator, EntitiesWidget)();
}


}

}