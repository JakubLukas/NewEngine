#include "plugin.h"

#include "core/engine.h"
#include "core/asserts.h"
#include "core/string.h"


namespace Veng
{


struct PluginEntry
{
	const char* name;
	IPlugin* (*createFunc)(Engine& engine);
};


static unsigned pluginsCount = 0;
static PluginEntry plugins[MAX_PLUGINS];


void RegisterPlugin(const char* name, IPlugin* (*createFunc)(Engine& engine))
{
	ASSERT(pluginsCount < MAX_PLUGINS);
	if (pluginsCount == MAX_PLUGINS)
		return;

	plugins[pluginsCount] = { name, createFunc };
	++pluginsCount;
}


static IPlugin* ConstructPlugin(const char* name, Engine& engine)
{
	for (unsigned i = 0; i < pluginsCount; ++i)
	{
		if (StrEqual(plugins[i].name, name))
			return plugins[i].createFunc(engine);
	}

	ASSERT2(false, "Plugin isn't registered");
	return nullptr;
}


}
