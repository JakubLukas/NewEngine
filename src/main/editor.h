#pragma once

#include "core/int.h"
#include "app.h"


namespace Veng
{

class IAllocator;
class Engine;


class Editor
{
public:
	static Editor* Create(IAllocator& allocator, App& app);
	static void Destroy(Editor* editor, IAllocator& allocator);

public:
	virtual ~Editor() {}

	virtual void Init() = 0;
	virtual void Update() = 0;
	virtual void Deinit() = 0;

	virtual void Resize(u32 width, u32 height) = 0;

	virtual Engine* GetEngine() = 0;//////////// tmp function

};

}