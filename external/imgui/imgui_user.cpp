#include "imgui.h"
#include "imgui_user.h"

#include "imgui_internal.h"


namespace ImGui
{

void PushItemsDisabled()
{
	PushItemFlag(ImGuiItemFlags_Disabled, true);
	PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
}

void PopItemsDisabled()
{
	ImGui::PopItemFlag();
	ImGui::PopStyleVar();
}

}