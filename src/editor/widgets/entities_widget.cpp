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


void EntitiesWidget::Init(Engine& engine, EditorInterface& editor)
{
	m_engine = &engine;

	if (m_engine->GetWorldCount() > 0)
		m_world = &m_engine->GetWorlds()[0];
}


void EntitiesWidget::Deinit()
{}


void EntitiesWidget::Update(EventQueue& queue)
{
	for(size_t i = 0; i < queue.GetPullEventsSize(); ++i)
	{
		const Event* event = queue.PullEvents()[i];
		switch (event->type)
		{
		case EventType::SelectWorld:
		{
			const EventSelectWorld* eventWorld = (EventSelectWorld*)event;
			m_world = m_engine->GetWorld(eventWorld->id);
		}
		break;
		case EventType::SelectEntity:
		{
			const EventSelectEntity* eventEntity = (EventSelectEntity*)event;
			m_selected = eventEntity->entity;
		}
		break;
		}
	}
}


void EntitiesWidget::Render(EventQueue& queue)
{
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::MenuItem("Add world")) m_world = m_engine->GetWorld(m_engine->AddWorld());
		if (m_world == nullptr) ImGui::PushItemsDisabled();
		if (ImGui::MenuItem("Create Entity")) m_world->CreateEntity();//todo: make button disabled, do not hide
		if (m_world == nullptr) ImGui::PopItemsDisabled();
		ImGui::EndMenuBar();
	}

	size_t worldsCount = m_engine->GetWorldCount();
	World* worlds = m_engine->GetWorlds();

	char buffer[64] = "No worlds to select";
	if (worldsCount > 0)
		ImFormatString(buffer, 64, "World %i", m_world->GetId());

	if (ImGui::BeginCombo("World", buffer))
	{
		for (int i = 0; i < worldsCount; ++i)
		{
			ImFormatString(buffer, 64, "World %i", worlds[i].GetId());
			if (ImGui::Selectable(buffer, worlds[i].GetId() == m_world->GetId())) {
				m_world = &worlds[i];
				queue.PushEvent(EventSelectWorld(m_world->GetId()));
			}
		}
		ImGui::EndCombo();
	}

	if (ImGui::Button("Create entity"))
		m_world->CreateEntity();

	ImGui::Separator();

	if(!ImGui::ListBoxHeader("##empty", ImGui::GetContentRegionAvail()))
	{
		ImGui::Text("Error: ListBoxHeader wasn't created");
		ImGui::PopItemWidth();
		return;
	}

	World::EntityIterator& entityIter = m_world->GetEntities();
	Entity deffered_delete_entity = INVALID_ENTITY;
	Entity entity;
	for(int i = 0; entityIter.GetNext(entity); ++i)
	{
		char item_text[64] = { '\0' };
		ImFormatString(item_text, 64, "Entity %i (id:%i)", i, (u64)entity);

		ImGui::PushID(i);
		if(ImGui::Selectable(item_text, entity == m_selected))
		{
			m_selected = entity;
			queue.PushEvent(EventSelectEntity(m_world->GetId(), m_selected));
		}

		if (ImGui::BeginPopupContextItem())
		{
			if (ImGui::Selectable("Remove"))
				deffered_delete_entity = entity;

			ImGui::EndPopup();
		}

		ImGui::PopID();
	}

	if (deffered_delete_entity != INVALID_ENTITY) {
		if (m_selected == deffered_delete_entity)
			m_selected = INVALID_ENTITY;
		m_world->DestroyEntity(deffered_delete_entity);
	}

	ImGui::ListBoxFooter();
}


REGISTER_WIDGET(entities)
{
	return NEW_OBJECT(allocator, EntitiesWidget)();
}


}

}