#include "entity_widget.h"

#include "../external/imgui/imgui.h"
#include "core/engine.h"
#include "core/matrix.h"

namespace Veng
{

class Engine;

namespace Editor
{


void EntityWidget::SetEngine(Engine* engine) { m_engine = engine; }

void EntityWidget::SetEntity(Entity entity) { m_entity = entity; }


void EntityWidget::RenderInternal()
{
	if(m_entity == INVALID_ENTITY)
		return;

	Transform entityTrans = m_engine->GetWorld((worldId)0)->GetEntityTransform(m_entity);
	ImGui::Text("Position: %f, %f, %f", entityTrans.position.x, entityTrans.position.y, entityTrans.position.z);
}


}

}