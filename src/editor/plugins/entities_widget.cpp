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
}


}

}