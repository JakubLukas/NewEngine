#include "entity_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "core/engine.h"
#include "core/world/world.h"
#include "core/math/matrix.h"
#include "core/math/math.h"
#include "core/math/math_ext.h"

#include "renderer/renderer.h"///////////////

#include "core/resource/resource_management.h"///////////
#include "core/resource/resource_manager.h"

#include "resource_widgets.h"


namespace Veng
{

class Engine;

namespace Editor
{


void EntityWidget::Init(Engine& engine, EditorInterface& editor)
{
	m_engine = &engine;
	m_editorInterface = &editor;
}


void EntityWidget::Deinit()
{}


void EntityWidget::Update(EventQueue& queue)
{
	for(size_t i = 0; i < queue.GetPullEventsSize(); ++i)
	{
		const Event* event = queue.PullEvents()[i];
		if(event->type == EventType::SelectWorld)
		{
			const EventSelectWorld* eventWorld = (EventSelectWorld*)event;
			m_world = m_engine->GetWorld(eventWorld->id);
			m_entity = INVALID_ENTITY;
		}
		else if(event->type == EventType::SelectEntity)
		{
			const EventSelectEntity* eventEntity = (EventSelectEntity*)event;
			m_world = m_engine->GetWorld(eventEntity->worldId);
			m_entity = eventEntity->entity;
		}
	}
}


void EntityWidget::Render(EventQueue& queue)
{
	if (m_world == nullptr)
	{
		ImGui::Text("No World selected");
		return;
	}
	if (m_entity == INVALID_ENTITY)
	{
		ImGui::Text("No Entity selected");
		return;
	}
	if (!m_world->ExistsEntity(m_entity))
	{
		m_entity = INVALID_ENTITY;
		ImGui::Text("No Entity selected");
		return;
	}

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		Transform& entityTrans = m_world->GetEntityTransform(m_entity);
		ImGui::InputFloat3("position", &entityTrans.position.x, "%.2f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);

		Vector3 eulerAngles;
		QuatToEuler(entityTrans.rotation, eulerAngles);
		ToDeg(eulerAngles);
		if(ImGui::InputFloat3("rotation", &eulerAngles.x, "%.2f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue))
		{
			ToRad(eulerAngles);
			EulerToQuat(eulerAngles, entityTrans.rotation);
		}

		ImGui::InputFloat("scale", &entityTrans.scale, 0.1f, 1.0f, "%.2f", ImGuiInputTextFlags_AutoSelectAll | ImGuiInputTextFlags_EnterReturnsTrue);

		ImGui::Separator();
	}

	System* const* systems = m_engine->GetSystems();
	for (size_t i = 0; i < m_engine->GetSystemCount(); ++i)
	{
		System* system = systems[i];
		Scene* scene = system->GetScene(m_world->GetId());

		if (scene == nullptr) continue;

		SceneEditor* editor = system->GetEditor();

		uint count;
		const ComponentBase** components = system->GetComponents(count);
		for (size_t j = 0; j < count; ++j)
		{
			if (components[j]->Has(*scene, m_entity))
			{
				bool result = ImGui::CollapsingHeader(components[j]->name, ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_AllowItemOverlap);
				ImGui::SameLine(ImGui::GetContentRegionAvail().x - 3);
				if (ImGui::Button("x")) {
					components[j]->Destroy(*scene, m_entity);
					result = false;
				}
				if (result)
				{
					editor->EditComponent(*m_editorInterface, *components[j], m_world->GetId(), m_entity);
					ImGui::Separator();
				}
			}
		}
	}

	ImGui::Button("Add component");
	if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonLeft))
	{
		for (size_t i = 0; i < m_engine->GetSystemCount(); ++i)
		{
			System* system = systems[i];

			uint count;
			const ComponentBase** components = system->GetComponents(count);

			for (size_t j = 0; j < count; ++j)
			{
				if (ImGui::Selectable(components[j]->name))
					components[j]->Create(*system->GetScene(m_world->GetId()), m_entity);
			}
		}
		ImGui::EndPopup();
	}
}


REGISTER_WIDGET(EntityWidget)
{
	return NEW_OBJECT(allocator, EntityWidget)();
}


}

}
