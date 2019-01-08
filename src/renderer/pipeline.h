#pragma once


namespace Veng
{

class IAllocator;
class Renderer;


class Pipeline
{
public:
	static Pipeline* Create(IAllocator& allocator, Renderer& renderer);
	static void Destroy(Pipeline* pipeline);

public:
	virtual ~Pipeline() {}

	virtual void Load() = 0;
	virtual void Render() = 0;

};


}