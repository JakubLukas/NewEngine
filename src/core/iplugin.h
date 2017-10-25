#pragma once


namespace Veng
{


class Engine;


class IPlugin
{
public:
	virtual ~IPlugin() {}

	virtual void Update(float deltaTime) = 0;
	virtual const char* GetName() const = 0;
};


}