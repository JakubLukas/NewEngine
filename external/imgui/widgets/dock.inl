// https://github.com/vassvik/imgui_docking_minimal/
//
// This is free and unencumbered software released into the public domain.
//
// Anyone is free to copy, modify, publish, use, compile, sell, or
// distribute this software, either in source code form or as a compiled
// binary, for any purpose, commercial or non-commercial, and by any
// means.
//
// In jurisdictions that recognize copyright laws, the author or authors
// of this software dedicate any and all copyright interest in the
// software to the public domain. We make this dedication for the benefit
// of the public at large and to the detriment of our heirs and
// successors. We intend this dedication to be an overt act of
// relinquishment in perpetuity of all present and future rights to this
// software under copyright law.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//
// For more information, please refer to <http://unlicense.org>

#include "../imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "../imgui_internal.h"

#include "core/file/blob.h"

namespace ImGui
{

struct NewPlaceholder { };

}

void* operator new(size_t size, ImGui::NewPlaceholder, void* where)
{
	return where;
}


namespace ImGui
{

struct DockContext
{
	enum Slot_
	{
		Slot_Left,
		Slot_Right,
		Slot_Top,
		Slot_Bottom,
		Slot_Tab,

		Slot_Float,
		Slot_None
	};

	enum EndAction_
	{
		EndAction_None,
		EndAction_Panel,
		EndAction_End,
		EndAction_EndChild
	};

	enum Status_
	{
		Status_Docked,
		Status_Float,
		Status_Dragged
	};

	static const int INVALID_INDEX = -1;

	struct Dock
	{
		ImU32   id = 0;
		char*   label = NULL;
		int     next_tab = INVALID_INDEX;
		int     prev_tab = INVALID_INDEX;
		int     parent = INVALID_INDEX;
		int     self = INVALID_INDEX;
		ImVec2  pos = { 0, 0 };
		ImVec2  size = { 1, 1 };
		bool    active = true;
		Status_ status = Status_Float;
		bool    opened = false;

		int     children[2] = { INVALID_INDEX, INVALID_INDEX };
		char    location[16] = { 0 };
		int     last_frame = 0;
		int     invalid_frames = 0;
		bool    first = true;
	};






	ImVec2 dockGetMinSize(int dockIdx) const
	{
		IM_ASSERT(INVALID_INDEX != dockIdx);
		const Dock& dock = m_docks[dockIdx];

		if (INVALID_INDEX == dock.children[0])
			return ImVec2(16, 16 + GetTextLineHeightWithSpacing());

		ImVec2 s0 = dockGetMinSize(dock.children[0]);
		ImVec2 s1 = dockGetMinSize(dock.children[1]);

		if (dockIsHorizontal(dockIdx))
			return ImVec2(s0.x + s1.x, ImMax(s0.y, s1.y));
		else
			return ImVec2(ImMax(s0.x, s1.x), s0.y + s1.y);
	}


	bool dockIsHorizontal(int dockIdx) const
	{
		IM_ASSERT(INVALID_INDEX != dockIdx);
		const Dock& dock = m_docks[dockIdx];

		const Dock& childOne = m_docks[dock.children[0]];
		const Dock& childTwo = m_docks[dock.children[1]];

		return childOne.pos.x < childTwo.pos.x;
	}


	void dockSetParent(int dockIdx, int parentIdx)
	{
		IM_ASSERT(INVALID_INDEX != dockIdx);
		Dock& dock = m_docks[dockIdx];

		dock.parent = parentIdx;

		for (int tmpIdx = dock.prev_tab; INVALID_INDEX != tmpIdx; tmpIdx = m_docks[tmpIdx].prev_tab)
			m_docks[tmpIdx].parent = parentIdx;
		for (int tmpIdx = dock.next_tab; INVALID_INDEX != tmpIdx; tmpIdx = m_docks[tmpIdx].next_tab)
			m_docks[tmpIdx].parent = parentIdx;
	}


	int dockGetSibling(int dockIdx)
	{
		IM_ASSERT(INVALID_INDEX != dockIdx);
		const Dock& dock = m_docks[dockIdx];

		IM_ASSERT(INVALID_INDEX != dock.parent);
		const Dock& parent = m_docks[dock.parent];

		if (parent.children[0] == dockGetFirstTab(dockIdx))
			return parent.children[1];

		return parent.children[0];
	}


	int dockGetFirstTab(int dockIdx)
	{
		IM_ASSERT(INVALID_INDEX != dockIdx);

		int idx = dockIdx;
		while (INVALID_INDEX != m_docks[idx].prev_tab)
			idx = m_docks[idx].prev_tab;

		return idx;
	}


	void dockSetActive(int dockIdx)
	{
		IM_ASSERT(INVALID_INDEX != dockIdx);
		Dock& dock = m_docks[dockIdx];

		dock.active = true;
		for (int tmpIdx = dock.prev_tab; INVALID_INDEX != tmpIdx; tmpIdx = m_docks[tmpIdx].prev_tab)
			m_docks[tmpIdx].active = false;
		for (int tmpIdx = dock.next_tab; INVALID_INDEX != tmpIdx; tmpIdx = m_docks[tmpIdx].next_tab)
			m_docks[tmpIdx].active = false;
	}


	bool dockIsContainer(int dockIdx) const
	{
		IM_ASSERT(INVALID_INDEX != dockIdx);
		const Dock& dock = m_docks[dockIdx];

		return INVALID_INDEX != dock.children[0];
	}


	void dockSetChildrenPosSize(int dockIdx, const ImVec2& pos, const ImVec2& size)
	{
		IM_ASSERT(INVALID_INDEX != dockIdx);
		const Dock& dock = m_docks[dockIdx];

		const Dock& childOne = m_docks[dock.children[0]];
		const Dock& childTwo = m_docks[dock.children[1]];

		ImVec2 minSizeOne = dockGetMinSize(dock.children[0]);
		ImVec2 minSizeTwo = dockGetMinSize(dock.children[1]);

		ImVec2 p = pos;
		ImVec2 s = childOne.size;

		if (dockIsHorizontal(dockIdx))
		{
			s.y = size.y;
			s.x = (float)int((size.x * childOne.size.x) / (childOne.size.x + childTwo.size.x));
			s.x = ImMax(minSizeOne.x, s.x);
			dockSetPosSize(dock.children[0], p, s);

			p.x += s.x;
			s.x = size.x - s.x;
			s.x = ImMax(minSizeTwo.x, s.x);
			dockSetPosSize(dock.children[1], p, s);
		}
		else
		{
			s.x = size.x;
			s.y = (float)int((size.y * childOne.size.y) / (childOne.size.y + childTwo.size.y));
			s.y = ImMax(minSizeOne.y, s.y);
			dockSetPosSize(dock.children[0], p, s);

			p.y += s.y;
			s.y = size.y - s.y;
			s.y = ImMax(minSizeTwo.y, s.y);
			dockSetPosSize(dock.children[1], p, s);
		}
	}

	void dockSetPosSize(int dockIdx, const ImVec2& pos, const ImVec2& size)
	{
		IM_ASSERT(INVALID_INDEX != dockIdx);
		Dock& dock = m_docks[dockIdx];

		dock.size = size;
		dock.pos = pos;
		for (int tmpIdx = dock.prev_tab; INVALID_INDEX != tmpIdx; tmpIdx = m_docks[tmpIdx].prev_tab)
		{
			m_docks[tmpIdx].size = size;
			m_docks[tmpIdx].pos = pos;
		}
		for (int tmpIdx = dock.next_tab; INVALID_INDEX != tmpIdx; tmpIdx = m_docks[tmpIdx].next_tab)
		{
			m_docks[tmpIdx].size = size;
			m_docks[tmpIdx].pos = pos;
		}

		if (dockIsContainer(dockIdx))
			dockSetChildrenPosSize(dockIdx, pos, size);
	}







	ImVector<Dock> m_docks;
	ImVec2 m_drag_offset;
	int m_current = INVALID_INDEX;
	int m_to_remove = INVALID_INDEX;
	int m_last_frame = 0;
	EndAction_ m_end_action;


	~DockContext()
	{
		for (Dock& item : m_docks)
		{
			MemFree(item.label);
		}
	}


	Dock& getDock(const char* label, bool opened)
	{
		ImU32 id = ImHash(label, 0);
		for (int i = 0; i < m_docks.size(); ++i)
		{
			if (m_docks[i].id == id)
			{
				return m_docks[i];
			}
		}

		m_docks.push_back(Dock());
		Dock& new_dock = m_docks.back();
		new_dock.self = m_docks.size() - 1;
		new_dock.label = ImStrdup(label);
		IM_ASSERT(new_dock.label);
		new_dock.id = id;
		dockSetActive(new_dock.self);
		new_dock.status = Status_Float;
		new_dock.pos = ImVec2(0, 0);
		new_dock.size = GetIO().DisplaySize;
		new_dock.opened = opened;
		new_dock.first = true;
		new_dock.last_frame = 0;
		new_dock.invalid_frames = 0;
		new_dock.location[0] = 0;
		return new_dock;
	}


	/*void putInBackground()
	{
		ImGuiWindow* win = GetCurrentWindow();
		ImGuiContext& g = *GImGui;
		if (g.Windows[0] == win)
			return;

		for (int i = 0; i < g.Windows.Size; i++)
		{
			if (g.Windows[i] == win)
			{
				for (int j = i - 1; j >= 0; --j)
				{
					g.Windows[j + 1] = g.Windows[j];
				}
				g.Windows[0] = win;
				break;
			}
		}
	}*/

	void splits()
	{
		if (GetFrameCount() == m_last_frame)
			return;

		m_last_frame = GetFrameCount();

		ImU32 color = GetColorU32(ImGuiCol_Button);
		ImU32 color_hovered = GetColorU32(ImGuiCol_ButtonHovered);
		ImDrawList* draw_list = GetWindowDrawList();
		ImGuiIO& io = GetIO();
		for (int i = 0; i < m_docks.size(); ++i)
		{
			Dock& dock = m_docks[i];
			if (!dockIsContainer(i))
				continue;

			PushID(i);
			if (!IsMouseDown(0))
			{
				dock.status = Status_Docked;
			}

			ImVec2 dsize(0, 0);
			SetCursorScreenPos(m_docks[dock.children[1]].pos);
			ImVec2 min_size0 = dockGetMinSize(dock.children[0]);
			ImVec2 min_size1 = dockGetMinSize(dock.children[1]);
			if (dockIsHorizontal(dock.self))
			{
				InvisibleButton("split", ImVec2(3, dock.size.y));
				if (dock.status == Status_Dragged)
					dsize.x = io.MouseDelta.x;
				dsize.x = -ImMin(-dsize.x, m_docks[dock.children[0]].size.x - min_size0.x);
				dsize.x = ImMin(dsize.x, m_docks[dock.children[1]].size.x - min_size1.x);
			}
			else
			{
				InvisibleButton("split", ImVec2(dock.size.x, 3));
				if (dock.status == Status_Dragged)
					dsize.y = io.MouseDelta.y;
				dsize.y = -ImMin(-dsize.y, m_docks[dock.children[0]].size.y - min_size0.y);
				dsize.y = ImMin(dsize.y, m_docks[dock.children[1]].size.y - min_size1.y);
			}
			ImVec2 new_size0 = m_docks[dock.children[0]].size + dsize;
			ImVec2 new_size1 = m_docks[dock.children[1]].size - dsize;
			ImVec2 new_pos1 = m_docks[dock.children[1]].pos + dsize;
			dockSetPosSize(dock.children[0], m_docks[dock.children[0]].pos, new_size0);
			dockSetPosSize(dock.children[1], new_pos1, new_size1);

			if (IsItemHovered() && IsMouseClicked(0))
			{
				dock.status = Status_Dragged;
			}

			draw_list->AddRectFilled(GetItemRectMin(), GetItemRectMax(), IsItemHovered() ? color_hovered : color);
			PopID();
		}
	}

	void checkNonexistent()
	{
		int frame_limit = ImMax(0, ImGui::GetFrameCount() - 2);
		for (int i = 0; i < m_docks.size(); ++i)
		{
			Dock& dock = m_docks[i];
			if (dockIsContainer(dock.self))
				continue;
			if (dock.status == Status_Float)
				continue;
			if (dock.last_frame < frame_limit)
			{
				++dock.invalid_frames;
				if (dock.invalid_frames > 2)
				{
					doUndock(dock);
					dock.status = Status_Float;
				}
				return;
			}
			dock.invalid_frames = 0;
		}
	}

	void beginPanel()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
			ImGuiWindowFlags_NoScrollWithMouse |
			ImGuiWindowFlags_NoBringToFrontOnFocus;
		int rootIdx = getRootDock();
		const ImVec2& displaySize = GetIO().DisplaySize;
		if (INVALID_INDEX != rootIdx)
		{
			const Dock& root = m_docks[rootIdx];
			const ImVec2 percentage(displaySize.x / root.size.x, displaySize.y / root.size.y);
			const ImVec2 rescaledPos = root.pos * percentage;
			const ImVec2 rescaledSize = root.size * percentage;
			SetNextWindowPos(rescaledPos);
			SetNextWindowSize(rescaledSize);
		}
		else
		{
			SetNextWindowPos(ImVec2(0, 0));
			SetNextWindowSize(displaySize);
		}
		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
		Begin("###DockPanel", NULL, flags);
		splits();

		checkNonexistent();
	}

	void endPanel()
	{
		End();
		ImGui::PopStyleVar();
	}

	// Doesn't use input??
	Dock* getDockAt(const ImVec2& /*pos*/)
	{
		for (int i = 0; i < m_docks.size(); ++i)
		{
			Dock& dock = m_docks[i];
			if (dockIsContainer(dock.self))
				continue;
			if (dock.status != Status_Docked)
				continue;
			if (IsMouseHoveringRect(dock.pos, dock.pos + dock.size, false))
			{
				return &dock;
			}
		}

		return NULL;
	}

	static ImRect getDockedRect(const ImRect& rect, Slot_ dock_slot)
	{
		ImVec2 half_size = rect.GetSize() * 0.5f;
		switch (dock_slot)
		{
		case Slot_Top: return ImRect(rect.Min, rect.Min + ImVec2(rect.Max.x, half_size.y));
		case Slot_Right: return ImRect(rect.Min + ImVec2(half_size.x, 0), rect.Max);
		case Slot_Bottom: return ImRect(rect.Min + ImVec2(0, half_size.y), rect.Max);
		case Slot_Left: return ImRect(rect.Min, rect.Min + ImVec2(half_size.x, rect.Max.y));
		default: return rect;
		}
	}

	static ImRect getSlotRect(ImRect parent_rect, Slot_ dock_slot)
	{
		ImVec2 size = parent_rect.Max - parent_rect.Min;
		ImVec2 center = parent_rect.Min + size * 0.5f;
		switch (dock_slot)
		{
		case Slot_Top: return ImRect(center + ImVec2(-20, -50), center + ImVec2(20, -30));
		case Slot_Right: return ImRect(center + ImVec2(30, -20), center + ImVec2(50, 20));
		case Slot_Bottom: return ImRect(center + ImVec2(-20, +30), center + ImVec2(20, 50));
		case Slot_Left: return ImRect(center + ImVec2(-50, -20), center + ImVec2(-30, 20));
		default: return ImRect(center - ImVec2(20, 20), center + ImVec2(20, 20));
		}
	}

	static ImRect getSlotRectOnBorder(ImRect parent_rect, Slot_ dock_slot)
	{
		ImVec2 size = parent_rect.Max - parent_rect.Min;
		ImVec2 center = parent_rect.Min + size * 0.5f;
		switch (dock_slot)
		{
		case Slot_Top:
			return ImRect(ImVec2(center.x - 20, parent_rect.Min.y + 10),
				ImVec2(center.x + 20, parent_rect.Min.y + 30));
		case Slot_Left:
			return ImRect(ImVec2(parent_rect.Min.x + 10, center.y - 20),
				ImVec2(parent_rect.Min.x + 30, center.y + 20));
		case Slot_Bottom:
			return ImRect(ImVec2(center.x - 20, parent_rect.Max.y - 30),
				ImVec2(center.x + 20, parent_rect.Max.y - 10));
		case Slot_Right:
			return ImRect(ImVec2(parent_rect.Max.x - 30, center.y - 20),
				ImVec2(parent_rect.Max.x - 10, center.y + 20));
		default: IM_ASSERT(false);
		}
		IM_ASSERT(false);
		return ImRect();
	}

	int getRootDock()
	{
		for (int i = 0; i < m_docks.size(); ++i)
		{
			if (INVALID_INDEX == m_docks[i].parent &&
				(m_docks[i].status == Status_Docked || INVALID_INDEX != m_docks[i].children[0]))
			{
				return i;
			}
		}
		return INVALID_INDEX;
	}

	bool dockSlots(Dock& dock, Dock* dest_dock, const ImRect& rect, bool on_border)
	{
		ImDrawList* canvas = GetWindowDrawList();
		ImU32 color = GetColorU32(ImGuiCol_Button);
		ImU32 color_hovered = GetColorU32(ImGuiCol_ButtonHovered);
		ImVec2 mouse_pos = GetIO().MousePos;
		for (int i = 0; i < (on_border ? 4 : 5); ++i)
		{
			ImRect r = on_border ? getSlotRectOnBorder(rect, (Slot_)i) : getSlotRect(rect, (Slot_)i);
			bool hovered = r.Contains(mouse_pos);

			canvas->AddRectFilled(r.Min, r.Max, hovered ? color_hovered : color);
			if (!hovered)
				continue;

			if (!IsMouseDown(0))
			{
				doDock(dock.self, dest_dock ? dest_dock->self : getRootDock(), (Slot_)i);
				return true;
			}
			ImRect docked_rect = getDockedRect(rect, (Slot_)i);
			canvas->AddRectFilled(docked_rect.Min, docked_rect.Max, GetColorU32(ImGuiCol_Button));
		}
		return false;
	}

	void handleDrag(Dock& dock)
	{
		Dock* dest_dock = getDockAt(GetIO().MousePos);

		Begin("##Overlay",
			NULL,
			ImGuiWindowFlags_Tooltip | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings |
			ImGuiWindowFlags_AlwaysAutoResize);
		ImDrawList* canvas = GetWindowDrawList();

		canvas->PushClipRectFullScreen();

		ImU32 docked_color = GetColorU32(ImGuiCol_FrameBg);
		docked_color = (docked_color & 0x00ffFFFF) | 0x80000000;
		dock.pos = GetIO().MousePos - m_drag_offset;
		if (NULL != dest_dock)
		{
			if (dockSlots(dock, dest_dock, ImRect(dest_dock->pos, dest_dock->pos + dest_dock->size), false))
			{
				canvas->PopClipRect();
				End();
				return;
			}
		}
		if (dockSlots(dock, NULL, ImRect(ImVec2(0, 0), GetIO().DisplaySize), true))
		{
			canvas->PopClipRect();
			End();
			return;
		}
		canvas->AddRectFilled(dock.pos, dock.pos + dock.size, docked_color);
		canvas->PopClipRect();

		if (!IsMouseDown(0))
		{
			dock.status = Status_Float;
			dock.location[0] = 0;
			dockSetActive(dock.self);
		}

		End();
	}

	void fillLocation(Dock& dock)
	{
		if (dock.status == Status_Float) return;
		char* c = dock.location;
		Dock* tmp = &dock;
		while (INVALID_INDEX != tmp->parent)
		{
			*c = getLocationCode(tmp);
			tmp = &m_docks[tmp->parent];
			++c;
		}
		*c = 0;
	}

	void doUndock(Dock& dock)
	{
		if (INVALID_INDEX != dock.prev_tab)
			dockSetActive(dock.prev_tab);
		else if (INVALID_INDEX != dock.next_tab)
			dockSetActive(dock.next_tab);
		else
			dock.active = false;

		if (INVALID_INDEX != dock.parent)
		{
			Dock& container = m_docks[dock.parent];
			Dock& sibling = m_docks[dockGetSibling(dock.self)];
			if (container.children[0] == dock.self)
			{
				container.children[0] = dock.next_tab;
			}
			else if (container.children[1] == dock.self)
			{
				container.children[1] = dock.next_tab;
			}

			bool remove_container = INVALID_INDEX == container.children[0] || INVALID_INDEX == container.children[1];
			if (remove_container)
			{
				if (INVALID_INDEX != container.parent)
				{
					if (m_docks[container.parent].children[0] == container.self)
						m_docks[container.parent].children[0] = sibling.self;
					else if (m_docks[container.parent].children[1] == container.self)
						m_docks[container.parent].children[1] = sibling.self;
					else
						IM_ASSERT(false);
					Dock& child = sibling;
					dockSetPosSize(child.self, container.pos, container.size);
					dockSetParent(child.self, container.parent);
				}
				else
				{
					if (INVALID_INDEX != container.children[0])
					{
						dockSetParent(container.children[0], INVALID_INDEX);
						dockSetPosSize(container.children[0], container.pos, container.size);
					}
					if (INVALID_INDEX != container.children[1])
					{
						dockSetParent(container.children[1], INVALID_INDEX);
						dockSetPosSize(container.children[1], container.pos, container.size);
					}
				}

				IM_ASSERT(INVALID_INDEX == m_to_remove);
				m_to_remove = container.self;
			}
		}
		if (INVALID_INDEX != dock.prev_tab)
			m_docks[dock.prev_tab].next_tab = dock.next_tab;
		if (INVALID_INDEX != dock.next_tab)
			m_docks[dock.next_tab].prev_tab = dock.prev_tab;
		dock.parent = INVALID_INDEX;
		dock.prev_tab = dock.next_tab = INVALID_INDEX;
	}

	void drawTabbarListButton(Dock& dock)
	{
		if (!dock.next_tab) return;

		ImDrawList* draw_list = GetWindowDrawList();
		if (InvisibleButton("list", ImVec2(16, 16)))
		{
			OpenPopup("tab_list_popup");
		}
		if (BeginPopup("tab_list_popup"))
		{
			int tmpIdx = dock.self;
			while (tmpIdx != INVALID_INDEX)
			{
				bool dummy = false;
				if (Selectable(m_docks[tmpIdx].label, &dummy))
				{
					dockSetActive(tmpIdx);
				}
				tmpIdx = m_docks[tmpIdx].next_tab;
			}
			EndPopup();
		}

		bool hovered = IsItemHovered();
		ImVec2 min = GetItemRectMin();
		ImVec2 max = GetItemRectMax();
		ImVec2 center = (min + max) * 0.5f;
		ImU32 text_color = GetColorU32(ImGuiCol_Text);
		ImU32 color_active = GetColorU32(ImGuiCol_FrameBgActive);
		draw_list->AddRectFilled(ImVec2(center.x - 4, min.y + 3),
			ImVec2(center.x + 4, min.y + 5),
			hovered ? color_active : text_color);
		draw_list->AddTriangleFilled(ImVec2(center.x - 4, min.y + 7),
			ImVec2(center.x + 4, min.y + 7),
			ImVec2(center.x, min.y + 12),
			hovered ? color_active : text_color);
	}

	bool tabbar(Dock& dock, bool close_button)
	{
		float tabbar_height = 2 * GetTextLineHeightWithSpacing();
		ImVec2 size0(dock.size.x, tabbar_height);
		bool tab_closed = false;

		SetCursorScreenPos(dock.pos);
		char tmp[20];
		ImFormatString(tmp, IM_ARRAYSIZE(tmp), "tabs%d", (int)dock.id);
		if (BeginChild(tmp, size0, true))
		{
			int dockTabIdx = dock.self;

			ImDrawList* draw_list = GetWindowDrawList();
			ImU32 color = GetColorU32(ImGuiCol_FrameBg);
			ImU32 color_active = GetColorU32(ImGuiCol_FrameBgActive);
			ImU32 color_hovered = GetColorU32(ImGuiCol_FrameBgHovered);
			ImU32 text_color = GetColorU32(ImGuiCol_Text);
			ImU32 text_color_disabled = GetColorU32(ImGuiCol_TextDisabled);
			float line_height = GetTextLineHeightWithSpacing();
			float tab_base;

			drawTabbarListButton(dock);

			while (dockTabIdx != INVALID_INDEX)
			{
				Dock& dock_tab = m_docks[dockTabIdx];
				SameLine(0, 15);

				const char* text_end = FindRenderedTextEnd(dock_tab.label);
				ImVec2 size1(CalcTextSize(dock_tab.label, text_end).x, line_height);
				if (InvisibleButton(dock_tab.label, size1))
				{
					dockSetActive(dockTabIdx);
				}

				if (IsItemActive() && IsMouseDragging())
				{
					m_drag_offset = GetMousePos() - dock_tab.pos;
					doUndock(dock_tab);
					dock_tab.status = Status_Dragged;
				}

				bool hovered = IsItemHovered();
				ImVec2 pos = GetItemRectMin();
				size1.x += 20 + GetStyle().ItemSpacing.x;

				tab_base = pos.y;

				draw_list->AddRectFilled(pos + ImVec2(-8.0f, 0.0),
					pos + size1,
					hovered ? color_hovered : (dock_tab.active ? color_active : color));
				draw_list->AddText(pos, text_color, dock_tab.label, text_end);

				if (dock_tab.active && close_button)
				{
					SameLine();
					tab_closed = InvisibleButton("close", ImVec2(16, 16));

					ImVec2 center = ((GetItemRectMin() + GetItemRectMax()) * 0.5f);
					draw_list->AddLine(center + ImVec2(-3.5f, -3.5f), center + ImVec2(3.5f, 3.5f), text_color);
					draw_list->AddLine(center + ImVec2(3.5f, -3.5f), center + ImVec2(-3.5f, 3.5f), text_color);
				}
				else {
					if (!dock_tab.active && close_button) {
						SameLine();
						InvisibleButton("close", ImVec2(16, 16));

						ImVec2 center = ((GetItemRectMin() + GetItemRectMax()) * 0.5f);
						draw_list->AddLine(center + ImVec2(-3.5f, -3.5f), center + ImVec2(3.5f, 3.5f), text_color_disabled);
						draw_list->AddLine(center + ImVec2(3.5f, -3.5f), center + ImVec2(-3.5f, 3.5f), text_color_disabled);
					}
				}

				dockTabIdx = m_docks[dockTabIdx].next_tab;
			}
			ImVec2 cp(dock.pos.x, tab_base + line_height);
			draw_list->AddLine(cp, cp + ImVec2(dock.size.x, 0), color);
		}
		EndChild();
		return tab_closed;
	}

	void setDockPosSize(Dock& dest, Dock& dock, Slot_ dock_slot, Dock& container)
	{
		IM_ASSERT(INVALID_INDEX == dock.prev_tab && INVALID_INDEX == dock.next_tab && INVALID_INDEX == dock.children[0] && INVALID_INDEX == dock.children[1]);

		dest.pos = container.pos;
		dest.size = container.size;
		dock.pos = container.pos;
		dock.size = container.size;

		switch (dock_slot)
		{
		case Slot_Bottom:
			dest.size.y *= 0.5f;
			dock.size.y *= 0.5f;
			dock.pos.y += dest.size.y;
			break;
		case Slot_Right:
			dest.size.x *= 0.5f;
			dock.size.x *= 0.5f;
			dock.pos.x += dest.size.x;
			break;
		case Slot_Left:
			dest.size.x *= 0.5f;
			dock.size.x *= 0.5f;
			dest.pos.x += dock.size.x;
			break;
		case Slot_Top:
			dest.size.y *= 0.5f;
			dock.size.y *= 0.5f;
			dest.pos.y += dock.size.y;
			break;
		default: IM_ASSERT(false); break;
		}
		dockSetPosSize(dest.self, dest.pos, dest.size);

		if (m_docks[container.children[1]].pos.x < m_docks[container.children[0]].pos.x ||
			m_docks[container.children[1]].pos.y < m_docks[container.children[0]].pos.y)
		{
			int tmp = container.children[0];
			container.children[0] = container.children[1];
			container.children[1] = tmp;
		}
	}

	void doDock(int dockIdx, int destIdx, Slot_ dock_slot)
	{
		IM_ASSERT(INVALID_INDEX != dockIdx);
		IM_ASSERT(INVALID_INDEX == m_docks[dockIdx].parent);

		if (INVALID_INDEX == destIdx)
		{
			m_docks[dockIdx].status = Status_Docked;
			dockSetPosSize(dockIdx, ImVec2(0, 0), GetIO().DisplaySize);
		}
		else if (dock_slot == Slot_Tab)
		{
			Dock& dock = m_docks[dockIdx];
			Dock* tmp = &m_docks[destIdx];
			while (INVALID_INDEX != tmp->next_tab)
			{
				tmp = &m_docks[tmp->next_tab];
			}

			tmp->next_tab = dock.self;
			dock.prev_tab = tmp->self;
			dock.size = tmp->size;
			dock.pos = tmp->pos;
			dock.parent = tmp->parent;
			dock.status = Status_Docked;
		}
		else if (dock_slot == Slot_None)
		{
			m_docks[dockIdx].status = Status_Float;
		}
		else
		{
			m_docks.push_back(Dock());
			Dock& container = m_docks.back();
			Dock& dest = m_docks[destIdx];

			container.self = m_docks.size() - 1;
			container.children[0] = dockGetFirstTab(destIdx);
			container.children[1] = dockIdx;
			container.next_tab = INVALID_INDEX;
			container.prev_tab = INVALID_INDEX;
			container.parent = dest.parent;
			container.size = dest.size;
			container.pos = dest.pos;
			container.status = Status_Docked;
			container.label = ImStrdup("");

			if (INVALID_INDEX == dest.parent)
			{
			}
			else if (dockGetFirstTab(dest.self) == m_docks[dest.parent].children[0])
			{
				m_docks[dest.parent].children[0] = container.self;
			}
			else
			{
				m_docks[dest.parent].children[1] = container.self;
			}

			dockSetParent(dest.self, container.self);
			m_docks[dockIdx].parent = container.self;
			m_docks[dockIdx].status = Status_Docked;

			setDockPosSize(dest, m_docks[dockIdx], dock_slot, container);
		}
		dockSetActive(dockIdx);
	}

	void rootDock(const ImVec2& pos, const ImVec2& size)
	{
		int rootIdx = getRootDock();
		if (INVALID_INDEX == rootIdx)
			return;

		const Dock& root = m_docks[rootIdx];

		ImVec2 min_size = dockGetMinSize(root.self);
		ImVec2 requested_size = size;
		dockSetPosSize(root.self, pos, ImMax(min_size, requested_size));
	}

	void handleRemoveDock()
	{
		if (INVALID_INDEX != m_to_remove)
		{
			MemFree(m_docks[m_to_remove].label);

			m_docks.erase_unsorted(m_docks.begin() + m_to_remove);
			if (m_to_remove < m_docks.size())
			{
				Dock& changedDock = m_docks[m_to_remove];
				int oldIdx = changedDock.self;
				changedDock.self = m_to_remove;
				if (INVALID_INDEX != changedDock.prev_tab)
					m_docks[changedDock.prev_tab].next_tab = m_to_remove;
				if (INVALID_INDEX != changedDock.next_tab)
					m_docks[changedDock.next_tab].prev_tab = m_to_remove;

				{
					int tmpIdx = changedDock.children[0];
					while (INVALID_INDEX != tmpIdx)
					{
						m_docks[tmpIdx].parent = m_to_remove;
						tmpIdx = m_docks[tmpIdx].next_tab;
					}
				}

				{
					int tmpIdx = changedDock.children[1];
					while (INVALID_INDEX != tmpIdx)
					{
						m_docks[tmpIdx].parent = m_to_remove;
						tmpIdx = m_docks[tmpIdx].next_tab;
					}
				}
				if (INVALID_INDEX != changedDock.parent)
				{
					if (m_docks[changedDock.parent].children[0] == oldIdx)
						m_docks[changedDock.parent].children[0] = m_to_remove;
					else if (m_docks[changedDock.parent].children[1] == oldIdx)
						m_docks[changedDock.parent].children[1] = m_to_remove;
					else
					{
						IM_ASSERT(false);
					}
				}
			}
			m_to_remove = INVALID_INDEX;
		}
	}

	void setDockActive()
	{
		IM_ASSERT(INVALID_INDEX != m_current);
		if (INVALID_INDEX != m_current)
			dockSetActive(m_current);
	}

	static Slot_ getSlotFromLocationCode(char code)
	{
		switch (code)
		{
		case '1': return Slot_Left;
		case '2': return Slot_Top;
		case '3': return Slot_Bottom;
		default: return Slot_Right;
		}
	}

	char getLocationCode(Dock* dock)
	{
		if (NULL == dock)
			return '0';

		if (dockIsHorizontal(dock->parent))
		{
			if (dock->pos.x < m_docks[m_docks[dock->parent].children[0]].pos.x)
				return '1';
			if (dock->pos.x < m_docks[m_docks[dock->parent].children[1]].pos.x)
				return '1';
			return '0';
		}
		else
		{
			if (dock->pos.y < m_docks[m_docks[dock->parent].children[0]].pos.y)
				return '2';
			if (dock->pos.y < m_docks[m_docks[dock->parent].children[1]].pos.y)
				return '2';
			return '3';
		}
	}

	void tryDockToStoredLocation(Dock& dock)
	{
		if (dock.status == Status_Docked)
			return;
		if (dock.location[0] == 0)
			return;

		int tmpIdx = getRootDock();
		if (INVALID_INDEX != tmpIdx)
			return;

		int prevIdx = INVALID_INDEX;
		char* c = dock.location + strlen(dock.location) - 1;
		while (c >= dock.location)
		{
			Dock& dock = m_docks[tmpIdx];
			prevIdx = tmpIdx;
			tmpIdx = (*c == getLocationCode(&m_docks[dock.children[0]])) ? dock.children[0] : dock.children[1];
			if (INVALID_INDEX != tmpIdx) --c;
		}
		if (tmpIdx != tmpIdx && INVALID_INDEX != m_docks[tmpIdx].children[0])
			tmpIdx = m_docks[tmpIdx].parent;
		doDock(dock.self, INVALID_INDEX != tmpIdx ? tmpIdx : prevIdx, INVALID_INDEX != tmpIdx && INVALID_INDEX != m_docks[tmpIdx].children[0] ? Slot_Tab : getSlotFromLocationCode(*c));
	}

	bool begin(const char* label, bool* opened, ImGuiWindowFlags extra_flags)
	{
		Dock& dock = getDock(label, NULL == opened || *opened);
		int dockIdx = dock.self;
		if (!dock.opened && (NULL == opened || *opened))
			tryDockToStoredLocation(dock);
		dock.last_frame = ImGui::GetFrameCount();
		if (strcmp(dock.label, label) != 0)
		{
			MemFree(dock.label);
			dock.label = ImStrdup(label);
		}

		m_end_action = EndAction_None;

		if (dock.first && NULL != opened)
			*opened = dock.opened;
		dock.first = false;
		if (NULL != opened && !*opened)
		{
			if (dock.status != Status_Float)
			{
				fillLocation(dock);
				doUndock(dock);
				dock.status = Status_Float;
			}
			dock.opened = false;
			return false;
		}
		dock.opened = true;

		m_end_action = EndAction_Panel;
		beginPanel();

		m_current = dock.self;
		if (dock.status == Status_Dragged)
			handleDrag(dock);

		dock = m_docks[dockIdx];///////////////////////////////////////////////////////
		bool is_float = (dock.status == Status_Float);

		if (is_float)
		{
			SetNextWindowPos(dock.pos);
			SetNextWindowSize(dock.size);
			bool ret = Begin(label,
				opened,
				ImGuiWindowFlags_NoCollapse | extra_flags);
			m_end_action = EndAction_End;
			dock.pos = GetWindowPos();
			dock.size = GetWindowSize();

			ImGuiContext& g = *GImGui;

			if (g.ActiveId == GetCurrentWindow()->MoveId && g.IO.MouseDown[0])
			{
				m_drag_offset = GetMousePos() - dock.pos;
				doUndock(dock);
				dock.status = Status_Dragged;
			}
			return ret;
		}

		if (!dock.active && dock.status != Status_Dragged) return false;

		m_end_action = EndAction_EndChild;

		PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
		PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
		float tabbar_height = GetTextLineHeightWithSpacing();
		if (tabbar(m_docks[dockGetFirstTab(dock.self)], NULL != opened))
		{
			fillLocation(dock);
			*opened = false;
		}
		ImVec2 pos = dock.pos;
		ImVec2 size = dock.size;
		pos.y += tabbar_height + GetStyle().WindowPadding.y;
		size.y -= tabbar_height + GetStyle().WindowPadding.y;

		SetCursorScreenPos(pos);
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
			ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus |
			extra_flags;
		char tmp[256];
		strcpy(tmp, label);
		strcat(tmp, "_docked"); // to avoid https://github.com/ocornut/imgui/issues/713
		bool ret = BeginChild(tmp, size, true, flags);
		PopStyleColor();
		PopStyleColor();
		return ret;
	}

	void end()
	{
		if (m_end_action == EndAction_End)
		{
			End();
		}
		else if (m_end_action == EndAction_EndChild)
		{
			PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
			PushStyleColor(ImGuiCol_BorderShadow, ImVec4(0, 0, 0, 0));
			EndChild();
			PopStyleColor();
			PopStyleColor();
		}
		m_current = INVALID_INDEX;
		if (m_end_action > EndAction_None)
			endPanel();
		handleRemoveDock();
	}

	int getDockIndex(Dock* dock)
	{
		if (NULL == dock)
			return -1;

		return dock->self;
	}

	Dock* getDockByIndex(int idx)
	{
		return (idx < 0 && idx >= m_docks.size()) ? NULL : &m_docks[idx];
	}

	void serialize(Veng::OutputBlob& blob)
	{
		int size = m_docks.size();
		blob.Write(size);
		blob.Write(m_docks.begin(), size * sizeof(Dock));
		for(int i = 0; i < size; ++i)
		{
			blob.WriteString(m_docks[i].label);
		}
	}

	void deserialize(Veng::InputBlob& blob)
	{
		int size;
		IM_ASSERT(blob.Read(size));
		m_docks.reserve(size);
		m_docks.resize(size);
		IM_ASSERT(blob.Read(m_docks.begin(), size * sizeof(Dock)));
		for(int i = 0; i < size; ++i)
		{
			static const size_t MAX_LABEL_SIZE = 256;
			char label[MAX_LABEL_SIZE];
			IM_ASSERT(blob.ReadString(label, MAX_LABEL_SIZE));
			m_docks[i].label = ImStrdup(label);
		}
	}
};


static DockContext* s_dock = NULL;

IMGUI_API void CreateDockContext()
{
	void* ptr = ImGui::MemAlloc(sizeof(DockContext));
	s_dock = new(NewPlaceholder(), ptr) DockContext();
}

IMGUI_API void DestroyDockContext()
{
	s_dock->~DockContext();
	ImGui::MemFree(s_dock);
	s_dock = NULL;
}

IMGUI_API void RootDock(const ImVec2& pos, const ImVec2& size)
{
	s_dock->rootDock(pos, size);
}

IMGUI_API bool BeginDock(const char* label, bool* opened, ImGuiWindowFlags extra_flags)
{
	return s_dock->begin(label, opened, extra_flags);
}

IMGUI_API void EndDock()
{
	s_dock->end();
}

IMGUI_API void SetDockActive()
{
	s_dock->setDockActive();
}

IMGUI_API void SerializeDock(Veng::OutputBlob& blob)
{
	s_dock->serialize(blob);
}

IMGUI_API void DeserializeDock(Veng::InputBlob& blob)
{
	s_dock->deserialize(blob);
}

} // namespace ImGui
