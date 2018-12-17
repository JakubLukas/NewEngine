#include "resource_manager_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "core/engine.h"
#include "core/os/os_utils.h"
#include "core/file/file.h"
#include "core/string.h"


namespace Veng
{


namespace Editor
{


ResourceManagerWidget::~ResourceManagerWidget()
{}


void ResourceManagerWidget::Init(IAllocator& allocator, Engine& engine)
{
	m_manager = engine.GetResourceManagement();
}


void ResourceManagerWidget::Deinit()
{}


void ResourceManagerWidget::Update(EventQueue& queue)
{}


void BuildFileBrowser(const Path& searchPath)
{
	FS::SearchInfo info;
	FS::searchHandle handle = FS::SearchFirstFile(searchPath, info);

	if (handle == FS::INVALID_SEARCH_HANDLE)
	{
		FS::SearchClose(handle);
		return;
	}

	do
	{
		if (string::Compare(info.fileName, ".") == 0)
			continue;
		if (string::Compare(info.fileName, "..") == 0)
			continue;

		bool isLeaf = (info.type == FS::SearchInfo::FileType::Normal);

		ImGuiTreeNodeFlags treeNodeFlags =
			ImGuiTreeNodeFlags_OpenOnArrow |
			ImGuiTreeNodeFlags_OpenOnDoubleClick;
		//if (isSelected)
		//treeNodeFlags |= ImGuiTreeNodeFlags_Selected;
		if (isLeaf)
			treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

		bool opened = ImGui::TreeNodeEx(info.fileName, treeNodeFlags, "%s (size:%d) (%d)",
			info.fileName, info.fileSize, info.modifiers);

		//if (ImGui::IsItemClicked())

		if (!opened)
			continue;

		if (!isLeaf)
		{
			StaticInputBuffer<Path::MAX_LENGTH> buffer;
			size_t curPathLen = string::Length(searchPath.path);
			buffer.Add(searchPath.path, curPathLen - 1);
			buffer << "/" << info.fileName << "/*";
			Path path(buffer.Cstr());
			BuildFileBrowser(path);
		}

		ImGui::TreePop();
	} while (FS::SearchNextFile(handle, info));

	FS::SearchClose(handle);
}


void ResourceManagerWidget::RenderInternal(EventQueue& queue)
{
	char buffer[Path::MAX_LENGTH] = { 0 };
	os::GetCurrentDir(buffer, Path::MAX_LENGTH);
	StaticInputBuffer<Path::MAX_LENGTH> pathBuf;
	pathBuf << buffer << "/*";
	Path path(pathBuf.Cstr());
	BuildFileBrowser(path);
}


REGISTER_WIDGET(resource_manager)
{
	return NEW_OBJECT(allocator, ResourceManagerWidget)();
}


}


}