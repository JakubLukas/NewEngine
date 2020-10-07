#pragma once

#include "core/allocator.h"


namespace Veng
{


struct ScriptClassBase
{
	virtual void Init(class Engine& engine) = 0;
	virtual void Deinit() = 0;
	virtual void Update(float deltaTime) = 0;
};


struct ScriptClassRegistry
{
	typedef ScriptClassBase* (*createFunction)(Allocator& allocator);

	ScriptClassRegistry(const char* name, createFunction creator);

	const char* name;
	u32 nameHash;
	createFunction creator;
	ScriptClassRegistry* next = nullptr;
};

ScriptClassRegistry* GetScriptRegistries();


#define REGISTER_SCRIPT(ClassName) \
	static ScriptClassBase* CreateWidget##ClassName(Allocator& allocator); \
	static ScriptClassRegistry s_registry##ClassName(#ClassName, CreateWidget##ClassName); \
	static ScriptClassBase* CreateWidget##ClassName(Allocator& allocator)


}