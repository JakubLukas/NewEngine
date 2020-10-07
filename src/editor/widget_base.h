#pragma once

#include "event_queue.h"

namespace Veng
{

class Engine;
class EditorInterface;


namespace Editor
{


class WidgetBase
{
public:
	virtual ~WidgetBase() {}
	virtual void Init(Engine& engine, EditorInterface& editor) = 0;
	virtual void Deinit() = 0;

	virtual void Update(EventQueue& queue) = 0;
	virtual void Render(EventQueue& queue) = 0;

	virtual const char* GetName() const = 0;
	virtual bool HasMenuBar() const { return false; }
};


}

}