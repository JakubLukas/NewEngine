#include "widget_register.h"


namespace Veng
{

namespace Editor
{


static WidgetRegistry* s_registriesList = nullptr;


WidgetRegistry::WidgetRegistry(createFunction creator)
	: creator(creator)
{
	next = s_registriesList;
	s_registriesList = this;
}


WidgetRegistry* GetWidgetRegistries()
{
	return s_registriesList;
}


}

}
