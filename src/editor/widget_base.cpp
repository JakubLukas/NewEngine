#include "widget_base.h"

#include "../external/imgui/imgui.h"


namespace Veng
{

namespace Editor
{


void WidgetBase::Render(EventQueue& queue)
{
	if (ImGui::BeginDock(GetName()))
	{
		RenderInternal(queue);
	}
	ImGui::EndDock();
}


}

}