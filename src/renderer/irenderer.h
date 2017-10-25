#pragma once

#include "core/iplugin.h"


namespace Veng
{

class IAllocator;
class Engine;


class RenderSystem : public IPlugin
{
public:
	static RenderSystem* Create(Engine& engine);
	static void Destroy(RenderSystem* system);

	virtual ~RenderSystem() override {}

	virtual void Update(float deltaTime) override = 0;
	virtual const char* GetName() const override = 0;

	virtual Engine& GetEngine() const = 0;
};


}