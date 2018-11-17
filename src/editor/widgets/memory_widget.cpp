#include "memory_widget.h"

#include "core/allocators.h"
#include "../external/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../external/imgui/imgui_internal.h"


namespace Veng
{


namespace Editor
{


void MemoryWidget::RenderInternal()
{
	for (size_t i = 0; i < GetAllocatorsSize(); ++i)
	{
		const IAllocator* a = GetAllocators()[i];
		ImGui::Text("Allocator #%d %s (c:%d , s:%d)", i, a->GetDebugName(), a->GetAllocCount(), a->GetAllocSize());
	}

	const IAllocator* allocator = GetAllocators()[1];

	ImVec2 pos = ImGui::GetCurrentWindow()->DC.CursorPos;
	ImVec2 size = ImGui::GetContentRegionAvail();
	ImDrawList* list = ImGui::GetWindowDrawList();
	list->AddRectFilled(pos, pos + size, ImColor(120, 120, 255, 255), 0);
	size_t pageSize = allocator->GetBlockSize();

	size_t allocIdx = 0;
	for (size_t i = 0, c = allocator->GetBlocksSize(); i < c; ++i)
	{
		uintptr page = (uintptr)allocator->GetBlocks()[i];
		uintptr allocStart = (uintptr)allocator->GetAllocations()[allocIdx];
		while (page <= allocStart && allocStart < page + pageSize && allocIdx < allocator->GetAllocationsSize())
		{
			size_t allocSize = allocator->GetSize((void*)allocStart);
			float start = (float)(allocStart - page) / pageSize * size.x;
			float end = (float)(allocStart + allocSize - page) / pageSize * size.x;

			ImGui::PushID((void*)allocStart);
			list->AddRectFilled(pos + ImVec2(start, (float)i / c * size.y), pos + ImVec2(end, (float)(i + 1) / c * size.y), ImColor(255, 120, 120, 255), 0);
			ImGui::PopID();

			allocStart = (uintptr)allocator->GetAllocations()[++allocIdx];
		}
	}
}


}


}