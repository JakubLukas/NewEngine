#pragma once


namespace Veng
{

class Allocator;

namespace Editor
{

class WidgetBase;


struct WidgetRegistry
{
	typedef WidgetBase*(*createFunction)(Allocator& allocator);

	WidgetRegistry(createFunction creator);

	createFunction creator;
	WidgetRegistry* next = nullptr;
};


WidgetRegistry* GetWidgetRegistries();


}

}


#define REGISTER_WIDGET(ClassName) \
	static WidgetBase* CreateWidget_##ClassName(Allocator& allocator); \
	static WidgetRegistry s_registry_##ClassName(CreateWidget_##ClassName); \
	static WidgetBase* CreateWidget_##ClassName(Allocator& allocator)
