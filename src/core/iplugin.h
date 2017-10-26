#pragma once

#include "int.h"


namespace Veng
{


class World;
enum class Entity : u32;


class IPlugin
{
public:
	virtual ~IPlugin() {}

	virtual void Update(float deltaTime) = 0;
	virtual const char* GetName() const = 0;
};


}