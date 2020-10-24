#pragma once

#include "../widget_base.h"


namespace ofbx { struct IScene; }

namespace Veng
{

class Engine;
class Allocator;

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

		Allocator* allocator;
		CoordSystemHandness coordSystem = CoordSystemHandness::Right;
		TriangleDefinition triangleDef = TriangleDefinition::ClockWise;
	};

public:
	ObjImportWidget(Allocator& allocator);
	~ObjImportWidget() override;
	void Init(Engine& engine, EditorInterface& editor) override;
	void Deinit() override;

	void Update(EventQueue& queue) override;
	void Render(EventQueue& queue) override;
	const char* GetName() const override { return "Object importer"; };

private:
	Allocator& m_allocator;
	ofbx::IScene* m_scene = nullptr;
	ConvertParams m_params;
};

}

}