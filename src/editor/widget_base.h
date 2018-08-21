#pragma once

namespace Veng
{

namespace Editor
{


class WidgetBase
{
public:
	void Render();

protected:
	virtual void RenderInternal() = 0;
	virtual const char* GetName() const = 0;
};


}

}