#pragma once

#include "../widget_base.h"
#include "core/file/path.h"
#include "core/containers/array.h"
#include "core/string.h"


namespace Veng
{

class ResourceManagement;


namespace Editor
{


class ResourceManagerWidget : public WidgetBase
{
public:
	ResourceManagerWidget(IAllocator& allocator);
	~ResourceManagerWidget() override;
	void Init(Engine& engine) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "ResourceManager"; };

private:
	void ProcessDirFiles(const Path& searchPath);
	void BuildFileBrowser(StaticInputBuffer<Path::MAX_LENGTH>& path, size_t itemIdx);

private:
	struct File
	{
		size_t firstChild = 0;
		size_t next = 0;
		char name[Path::MAX_LENGTH] = { 0 };
	};

private:
	IAllocator& m_allocator;
	Array<File> m_workingDirFiles;
};


}


}