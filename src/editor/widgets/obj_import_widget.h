#pragma once

#include "../widget_base.h"


namespace Veng
{

class Engine;
class IAllocator;

namespace Editor
{

class ObjImportWidget : public WidgetBase
{
public:
	ObjImportWidget(IAllocator& allocator);
	~ObjImportWidget() override;
	void Init(Engine& engine) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "ObjImporter"; };

private:
	IAllocator& m_allocator;
};

}

}