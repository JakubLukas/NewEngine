#pragma once


namespace Veng
{

class Allocator;
class Engine;
class RenderSystem;
class Path;


class Pipeline
{
public:
	static Pipeline* Create(Allocator& allocator, Engine& engine, RenderSystem& renderer);
	static void Destroy(Pipeline* pipeline);

public:
	virtual ~Pipeline() {}

	virtual void Load(const Path& path) = 0;
	virtual void Deinit() = 0;
	virtual void Render() = 0;

	virtual void* GetMainFrameBuffer() = 0;
};


}