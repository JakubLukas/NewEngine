#pragma once


namespace Veng
{


class Engine;


static const int MAX_PLUGINS = 32;


class IPlugin
{
public:
	virtual ~IPlugin() {}

	virtual void Update(float deltaTime) = 0;
	virtual const char* GetName() const = 0;
};


static void RegisterPlugin(const char* name, IPlugin* (*createFunc)(Engine& engine));


static IPlugin* ConstructPlugin(const char* name, Engine& engine);


}