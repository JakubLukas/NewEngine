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
	IMGUI_API void DockNewFrame();

	///
	IMGUI_API void DockRender();

	///
	IMGUI_API void SetDockActive();

} // namespace ImGui
