namespace Veng
{
class OutputBlob;
}

namespace ImGui
{
	///
	IMGUI_API void CreateDockContext();

	///
	IMGUI_API void DestroyDockContext();

	///
	IMGUI_API void RootDock(const ImVec2& pos, const ImVec2& size);

	///
	IMGUI_API bool BeginDock(const char* label, bool* opened = NULL, ImGuiWindowFlags extra_flags = 0);

	///
	IMGUI_API void EndDock();

	///
	IMGUI_API void SetDockActive();

	IMGUI_API void Serialize(Veng::OutputBlob& blob);

	IMGUI_API void Deserialize(Veng::InputBlob& blob);

} // namespace ImGui
