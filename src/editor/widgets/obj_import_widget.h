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
	struct ConvertParams
	{
		enum class CoordSystemHandness : u8
		{
			Right,
			Left,
		};
		enum class TriangleDefinition : u8
		{
			ClockWise,
			CounterClockWise,
		};

		IAllocator* allocator;
		CoordSystemHandness coordSystem = CoordSystemHandness::Right;
		TriangleDefinition triangleDef = TriangleDefinition::ClockWise;
	};

public:
	ObjImportWidget(IAllocator& allocator);
	~ObjImportWidget() override;
	void Init(Engine& engine, EditorInterface& editor) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;

protected:
	void RenderInternal(EventQueue& queue) override;
	const char* GetName() const override { return "ObjImporter"; };

private:
	IAllocator& m_allocator;
	ConvertParams m_params;
};

}

}