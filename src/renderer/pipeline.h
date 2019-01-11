#pragma once


namespace Veng
{

class IAllocator;
class Engine;
class RenderSystem;


class Pipeline
{
public:
	static Pipeline* Create(IAllocator& allocator, Engine& engine, RenderSystem& renderer);
	static void Destroy(Pipeline* pipeline);

public:
	virtual ~Pipeline() {}

	virtual void Init() = 0;
	virtual void Deinit() = 0;
	virtual void Render() = 0;

};


}