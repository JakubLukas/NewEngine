#pragma once

#include "core/resource/resource.h"


namespace Veng
{


class Command
{
public:
	virtual ~Command() {}
	virtual void Do() = 0;
	virtual void Undo() = 0;
};


class ChangeResourceCommand : public Command
{
public:
	//ChangeResourceCommand(resourceHandle original, resourceHandle)

private:
	resourceHandle handle;
};


}