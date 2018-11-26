#pragma once

namespace Veng
{

class IAllocator;
class Engine;


namespace Editor
{


class WidgetBase
{
public:
	virtual ~WidgetBase() {}
	virtual void Init(IAllocator& allocator, Engine& engine) = 0;
	void Render();

protected:
	virtual void RenderInternal() = 0;
	virtual const char* GetName() const = 0;
};


}

}