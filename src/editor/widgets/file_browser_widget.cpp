#include "file_browser_widget.h"

#include "../widget_register.h"
#include "../external/imgui/imgui.h"
#include "core/engine.h"
#include "core/os/os_utils.h"
#include "core/file/file.h"

#include "core/logs.h"///////////////////// TEMP


namespace Veng
{


namespace Editor
{


FileBrowserWidget::FileBrowserWidget(Allocator& allocator)
	: m_allocator(allocator)
	, m_workingDirFiles(m_allocator)
{
	m_workingDirFiles.Reserve(64);
	File& invalidFile = m_workingDirFiles.PushBack();
	memory::Copy(invalidFile.name, "INVALID FILE", 12);
}

FileBrowserWidget::~FileBrowserWidget()
{}

void FileBrowserWidget::ProcessDirFiles(const Path& searchPath)
{
	FS::SearchInfo info;
	FS::searchHandle handle = FS::SearchFirstFile(searchPath, info);

	if (handle == FS::INVALID_SEARCH_HANDLE)
	{
		FS::SearchClose(handle);
		return;
	}

	size_t parentIdx = m_workingDirFiles.GetSize() - 1;
	size_t prevIdx = 0;
	do
	{
		if (info.fileName[0] == '.') continue;

		bool isLeaf = (info.type == FS::SearchInfo::FileType::Normal);

		File& file = m_workingDirFiles.PushBack();
		memory::Copy(file.name, info.fileName, Path::MAX_LENGTH);
		if (m_workingDirFiles[parentIdx].firstChild == 0)
			m_workingDirFiles[parentIdx].firstChild = m_workingDirFiles.GetSize() - 1;
		else
			m_workingDirFiles[prevIdx].next = m_workingDirFiles.GetSize() - 1;

		prevIdx = m_workingDirFiles.GetSize() - 1;
		if (!isLeaf)
		{
			StaticInputBuffer<Path::MAX_LENGTH> buffer;
			size_t curPathLen = string::Length(searchPath.GetPath());
			buffer.Add(searchPath.GetPath(), curPathLen - 1);
			buffer << "/" << info.fileName << "/*";
			Path path(buffer.Cstr());

			ProcessDirFiles(path);
		}

	} while (FS::SearchNextFile(handle, info));

	FS::SearchClose(handle);
}

void FileBrowserWidget::Init(Engine& engine, EditorInterface& editor)
{
	char buffer[Path::BUFFER_LENGTH] = { 0 };
	os::GetWorkingDir(buffer, Path::BUFFER_LENGTH);

	StaticInputBuffer<Path::MAX_LENGTH> pathBuf;
	pathBuf << buffer << "/*";
	Path srcPath(pathBuf.Cstr());

	File& root = m_workingDirFiles.PushBack();
	memory::Copy(root.name, buffer, Path::MAX_LENGTH);

	ProcessDirFiles(srcPath);
}


void FileBrowserWidget::Deinit()
{}


void FileBrowserWidget::Update(EventQueue& queue)
{}


void FileBrowserWidget::BuildFileBrowser(StaticInputBuffer<Path::MAX_LENGTH>& path, size_t itemIdx)
{
	const File& item = m_workingDirFiles[itemIdx];

	bool isFile = item.firstChild == 0;

	ImGuiTreeNodeFlags treeNodeFlags =
		ImGuiTreeNodeFlags_OpenOnArrow |
		ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if (isFile)
		treeNodeFlags |= ImGuiTreeNodeFlags_Leaf;

	if (!ImGui::TreeNodeEx(item.name, treeNodeFlags, "%s", item.name))
		return;

	if (isFile)
	{
		const char* ext = string::FindCharR(item.name, '.') + 1;
		if (ext < item.name + Path::MAX_LENGTH)
		{
			if (ImGui::BeginDragDropSource())
			{
				size_t itemNameLen = string::Length(item.name);
				path << item.name;
				ImGui::SetDragDropPayload(ext, path.Cstr(), Path::MAX_LENGTH, ImGuiCond_Always);
				ImGui::Text(path.Cstr());
				ImGui::EndDragDropSource();
				path.Sub(itemNameLen);
			}
		}
	}
	else
	{
		size_t itemNameLen;
		size_t idx = item.firstChild;
		if (itemIdx != 1)
		{
			itemNameLen = string::Length(item.name);
			path << item.name << "/";
		}
		while (idx != 0)
		{
			BuildFileBrowser(path, idx);
			idx = m_workingDirFiles[idx].next;
		}
		if (itemIdx != 1)
			path.Sub(itemNameLen);
	}

	ImGui::TreePop();
}


void FileBrowserWidget::Render(EventQueue& queue)
{
	if (m_workingDirFiles.GetSize() < 2)
		return;

	StaticInputBuffer<Path::MAX_LENGTH> path;
	BuildFileBrowser(path, 1);
}


REGISTER_WIDGET(FileBrowserWidget)
{
	return NEW_OBJECT(allocator, FileBrowserWidget)(allocator);
}


}


}