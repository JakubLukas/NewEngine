#include "widget_base.h"

#include "../external/imgui/imgui.h"


namespace Veng
{

namespace Editor
{


void WidgetBase::Render()
{
	if (ImGui::BeginDock(GetName()))
	{
		RenderInternal();
	}
	ImGui::EndDock();
}


}

}