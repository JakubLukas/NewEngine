#include "memory_widget.h"

#include "core/allocators.h"
#include "../external/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../external/imgui/imgui_internal.h"


namespace Veng
{


namespace Editor
{


void BuildAllocatorTree(const Array<AllocatorDebugData>& allocators, const IAllocator* parent)
{
	for (int i = 0; i < allocators.GetSize(); ++i)
	{
		const AllocatorDebugData& allocData = allocators[i];

		if (allocData.parent != parent)
			continue;

		//const bool item_selected = (allocData.allocator == m_selected);

		char item_text[128] = { '\0' };
		ImFormatString(item_text, 128, "Allocator #%d %s (count:%d , size:%d)",
			i, allocData.allocator->GetDebugName(), allocData.allocator->GetAllocCount(), allocData.allocator->GetAllocSize());

		if (ImGui::TreeNode(item_text))
		{
			BuildAllocatorTree(allocators, allocData.allocator);
			ImGui::TreePop();
		}
	}
}

void MemoryWidget::RenderInternal()
{
	/*if (!ImGui::ListBoxHeader("##empty"))
	{
		ImGui::Text("Error: ListBoxHeader wasn't created");
		ImGui::PopItemWidth();
		return;
	}*/

	/*for (int i = 0; i < GetAllocators().GetSize(); ++i)
	{
		const AllocatorDebugData& allocData = GetAllocators()[i];

		const bool item_selected = (allocData.allocator == m_selected);

		char item_text[128] = { '\0' };
		ImFormatString(item_text, 128, "Allocator #%d %s (count:%d , size:%d)",
			i, allocData.allocator->GetDebugName(), allocData.allocator->GetAllocCount(), allocData.allocator->GetAllocSize());

		//ImGui::PushID(i);
		//if (ImGui::Selectable(item_text, item_selected))
		//	m_selected = allocData.allocator;
		//
		//if (item_selected)
		//	ImGui::SetItemDefaultFocus();
		//ImGui::PopID();
		if (ImGui::TreeNode(item_text))
		{
			ImGui::TreePop();
		}
	}*/

	//ImGui::ListBoxFooter();

	BuildAllocatorTree(GetAllocators(), nullptr);

	if (m_selected != nullptr)
	{
		ImVec2 pos = ImGui::GetCurrentWindow()->DC.CursorPos;
		ImVec2 size = ImGui::GetContentRegionAvail();
		ImDrawList* list = ImGui::GetWindowDrawList();
		list->AddRectFilled(pos, pos + size, ImColor(120, 120, 255, 255), 0);
		size_t pageSize = m_selected->GetBlockSize();

		size_t allocIdx = 0;
		for (size_t i = 0, c = m_selected->GetBlocksSize(); i < c; ++i)
		{
			uintptr page = (uintptr)m_selected->GetBlocks()[i];
			uintptr allocStart = (uintptr)m_selected->GetAllocations()[allocIdx];
			while (page <= allocStart && allocStart < page + pageSize && allocIdx < m_selected->GetAllocationsSize())
			{
				size_t allocSize = m_selected->GetSize((void*)allocStart);
				float start = (float)(allocStart - page) / pageSize * size.x;
				float end = (float)(allocStart + allocSize - page) / pageSize * size.x;

				ImGui::PushID((void*)allocStart);
				list->AddRectFilled(pos + ImVec2(start, (float)i / c * size.y), pos + ImVec2(end, (float)(i + 1) / c * size.y), ImColor(255, 120, 120, 255), 0);
				ImGui::PopID();

				allocStart = (uintptr)m_selected->GetAllocations()[++allocIdx];
			}
		}
	}
}


}


}