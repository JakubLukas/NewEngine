#include "memory_widget.h"

#include "core/allocators.h"
#include "../external/imgui/imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../external/imgui/imgui_internal.h"

#include "core/containers/hash_map.h"


namespace Veng
{


namespace Editor
{


struct MemoryWidgetAllocData
{
	ImColor color;
};

struct MemoryWidgetData
{
	MemoryWidgetData(IAllocator& allocator)
		: allocData(allocator, &HashPointer)
	{}

	HashMap<void*, MemoryWidgetAllocData> allocData;
	ImColor colors[4] = {
		ImColor(240, 120, 120, 255),
		ImColor(180, 120, 120, 255),
		ImColor(120, 240, 120, 255),
		ImColor(120, 180, 120, 255),
	};
};


MemoryWidget::~MemoryWidget()
{
	DELETE_OBJECT(*m_allocator, m_data);
}


void MemoryWidget::Init(IAllocator& allocator, Engine& engine)
{
	m_allocator = &allocator;
	m_data = NEW_OBJECT(allocator, MemoryWidgetData)(*m_allocator);
}


void BuildAllocatorTree(const Array<AllocatorDebugData>& allocators, const IAllocator* parent, const IAllocator*& selected)
{
	for (int i = 0; i < allocators.GetSize(); ++i)
	{
		const AllocatorDebugData& allocData = allocators[i];

		if (allocData.parent != parent)
			continue;

		const bool isSelected = (allocData.allocator == selected);

		bool isLeaf = true;
		for (int j = 0; j < allocators.GetSize(); ++j)
		{
			if (allocData.allocator == allocators[j].parent)
			{
				isLeaf = false;
				break;
			}
		}

		ImGuiTreeNodeFlags treeNodeFlags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick;
		if (isSelected)
			treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
		if (isLeaf)
			treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

		bool opened = ImGui::TreeNodeEx(allocData.allocator, treeNodeFlags, "Allocator #%d %s (count:%d , size:%d)",
			i, allocData.allocator->GetDebugName(), allocData.allocator->GetAllocCount(), allocData.allocator->GetAllocSize());

		if (ImGui::IsItemClicked())
			selected = allocData.allocator;

		if (!opened)
			continue;
		
		BuildAllocatorTree(allocators, allocData.allocator, selected);
		
		ImGui::TreePop();
	}
}

void MemoryWidget::RenderInternal()
{
	BuildAllocatorTree(GetAllocators(), nullptr, m_selected);

	if (m_selected != nullptr)
	{
		ImVec2 pos = ImGui::GetCursorPos();
		ImVec2 posDraw = pos + ImVec2(0, 2 * ImGui::GetTextLineHeight());
		ImVec2 size = ImGui::GetContentRegionAvail();
		ImDrawList* list = ImGui::GetWindowDrawList();
		list->AddRectFilled(posDraw, posDraw + size, ImColor(120, 120, 255, 255), 0);
		size_t blockSize = m_selected->GetBlockSize();

		size_t allocIdx = 0;
		for (size_t i = 0, c = m_selected->GetBlocksSize(); i < c; ++i)
		{
			uintptr block = (uintptr)m_selected->GetBlocks()[i];
			const AllocationDebugData* allocData = &m_selected->GetAllocations()[allocIdx];
			uintptr allocStart = (uintptr)allocData->allocation;

			size_t allocIdxPerBlock = 0;
			while (block <= allocStart && allocStart < block + blockSize && allocIdx < m_selected->GetAllocationsSize())
			{
				size_t allocSize = m_selected->GetSize((void*)allocStart);
				float start = (float)(allocStart - block) / blockSize * size.x;
				float end = (float)(allocStart + allocSize - block) / blockSize * size.x;

				const ImColor& color = m_data->colors[((i % 2) * 2) + (allocIdxPerBlock % 2)];
				ImGui::PushID((void*)allocStart);
				list->AddRectFilled(posDraw + ImVec2(start, (float)i / c * size.y), posDraw + ImVec2(end, (float)(i + 1) / c * size.y), color, 0);
				ImGui::SetCursorPos(pos + ImVec2(start, (float)i / c * size.y));
				ImGui::InvisibleButton("##tooltip", ImVec2(end - start, 1.0f / c * size.y));
				if(ImGui::IsItemHovered())
					ImGui::SetTooltip("address: %p, size: %d", allocStart, allocSize);
				ImGui::SetCursorPos(pos);
				ImGui::PopID();

				allocData = &m_selected->GetAllocations()[++allocIdx];
				allocIdxPerBlock++;
				allocStart = (uintptr)allocData->allocation;
			}
		}
	}
}


}


}