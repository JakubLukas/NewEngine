#include "model.h"


namespace Veng
{



struct PosColorVertex
{
	float m_x;
	float m_y;
	float m_z;
	uint32_t m_abgr;
};



bool Mesh::Load()
{
	m_vertex_decl
		.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
		.end();

	PosColorVertex cubeVertices[] =
	{
		{ -1.0f,  1.0f,  1.0f, 0xff000000 },
		{ 1.0f,  1.0f,  1.0f, 0xff0000ff },
		{ -1.0f, -1.0f,  1.0f, 0xff00ff00 },
		{ 1.0f, -1.0f,  1.0f, 0xff00ffff },
		{ -1.0f,  1.0f, -1.0f, 0xffff0000 },
		{ 1.0f,  1.0f, -1.0f, 0xffff00ff },
		{ -1.0f, -1.0f, -1.0f, 0xffffff00 },
		{ 1.0f, -1.0f, -1.0f, 0xffffffff },
	};

	const uint16_t cubeTriStrip[] =
	{
		0, 1, 2,
		3,
		7,
		1,
		5,
		0,
		4,
		2,
		6,
		7,
		4,
		5,
	};

	vertexBufferHandle = bgfx::createVertexBuffer(bgfx::copy(cubeVertices, sizeof(cubeVertices)), m_vertex_decl);
	indexBufferHandle = bgfx::createIndexBuffer(bgfx::copy(cubeTriStrip, sizeof(cubeTriStrip)));

	return true;
}


void Mesh::Clear()
{
	bgfx::destroy(vertexBufferHandle);
	bgfx::destroy(indexBufferHandle);
}


}