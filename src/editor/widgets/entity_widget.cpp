#include "entity_widget.h"

#include "../external/imgui/imgui.h"
#include "core/engine.h"

namespace Veng
{

class Engine;

namespace Editor
{


void EntityWidget::SetEngine(Engine* engine) { m_engine = engine; }


void EntityWidget::RenderInternal()
{
	ImGui::Text("No Entity selected");
}


}

}