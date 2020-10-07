#include "script_class.h"

#include "core/hashes.h"


namespace Veng
{


static ScriptClassRegistry* s_registriesList = nullptr;


ScriptClassRegistry::ScriptClassRegistry(const char* name, createFunction creator)
	: name(name)
	, nameHash(crc32_string(name))
	, creator(creator)
	, next(s_registriesList)
{
	s_registriesList = this;
}


ScriptClassRegistry* GetScriptRegistries()
{
	return s_registriesList;
}


}