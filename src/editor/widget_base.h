#pragma once

#include "event_queue.h"

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
	virtual void Init(Engine& engine) = 0;
	virtual void Deinit() = 0;

	virtual void Update(EventQueue& queue) = 0;
	void Render(EventQueue& queue);

protected:
	virtual void RenderInternal(EventQueue& queue) = 0;
	virtual const char* GetName() const = 0;
};


}

}