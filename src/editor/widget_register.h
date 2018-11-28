#pragma once


namespace Veng
{

class IAllocator;

namespace Editor
{

class WidgetBase;


struct WidgetRegistry
{
	typedef WidgetBase*(*createFunction)(IAllocator& allocator);

	WidgetRegistry(createFunction creator);

	createFunction creator;
	WidgetRegistry* next = nullptr;
};


WidgetRegistry* GetRegistries();


}

}


#define REGISTER_WIDGET(name) \
	static WidgetBase* CreateWidget(IAllocator& allocator); \
	static WidgetRegistry s_registry(CreateWidget); \
	static WidgetBase* CreateWidget(IAllocator& allocator)
